# apple-music-wrapper

> 把 Apple Music Android runtime 封进一个可复现的 `linux/arm64` 容器里，然后只把真正需要的能力露出来。

`apple-music-wrapper` 不是播放器，不是桌面客户端，也不是面向大众的 SDK。它是一个偏底层、偏工程化的包装层：

- 宿主机侧用 `./wrapper` 管理 `build`、`login`、`start` 和 `account`
- 容器里运行裁剪过的 Android 用户态与 Apple Music runtime
- 对外暴露三个接口：账号信息、`m3u8` 查询、音频样本解密

如果你要的是一个可脚本化、可持久化、尽量少做表面文章的 runtime wrapper，这个仓库就是干这个的。

## 这东西能做什么

- 登录 Apple ID，支持等待 `2fa.txt` 注入验证码
- 生成并导出 `storefront_id`、`dev_token`、`music_token`
- 为给定 `adam_id` 返回可播放或可下载的 `m3u8`
- 对 FairPlay 音频样本做按块解密
- 把运行期状态持久化到本地 `data/`

## 不做什么

- 不提供 UI
- 不负责曲库管理、下载调度、元数据整理
- 不承诺协议稳定，也不把内部二进制协议伪装成公共 API
- 不建议直接暴露到公网

## 快速开始

前提：

- Docker 可用
- 运行目标是 `linux/arm64`
- 推荐在 Apple Silicon 或原生 `arm64` Linux 上跑

首次构建：

```bash
./wrapper build
```

登录：

```bash
./wrapper login your@apple.id
```

`login` 会在终端里提示输入密码。如果账号触发 2FA，当前进程会等待 `data/2fa.txt` 出现。另开一个终端写入验证码即可：

```bash
./wrapper 2fa 123456
```

启动服务：

```bash
./wrapper start
```

查询账号状态：

```bash
./wrapper account
```

默认情况下，宿主机只会把端口绑定到 `127.0.0.1`。这是故意的，不建议改成公网地址。

## 对外接口

| 端口 | 协议 | 作用 |
| --- | --- | --- |
| `10020` | Raw TCP | 音频样本解密 |
| `20020` | Raw TCP | `adam_id -> m3u8` 查询 |
| `30020` | HTTP | 账号信息查询 |

### `30020` 账号接口

支持 `GET /` 或 `POST /`，返回 JSON：

```json
{
  "storefront_id": "...",
  "dev_token": "...",
  "music_token": "..."
}
```

示例：

```bash
curl -fsS http://127.0.0.1:30020/
```

### `20020` m3u8 接口

这是一个简单的二进制 TCP 协议，不是 HTTP。

请求格式：

```text
<u8 adam_id_len><adam_id_bytes>
```

响应格式：

```text
<m3u8_url>\n
```

如果没有结果，返回一个空行。运行期如果检测到离线下载能力可用，会优先走 download 流；否则走 playback lease 流。

### `10020` 解密接口

同样是二进制 TCP 协议。当前实现运行在小端 `arm64` 容器里，样本长度直接按本机 `uint32_t` 读取。

会话格式：

```text
<u8 adam_id_len><adam_id_bytes><u8 key_uri_len><key_uri_bytes>
```

随后可以重复发送多个样本块：

```text
<u32 sample_len><sample_bytes>
```

服务端会回写同长度的解密结果。发送 `sample_len = 0` 可以结束当前音轨上下文。

## 运行期数据

默认数据目录是仓库下的 [`data/`](data/)，容器内映射到：

```text
/data/data/com.apple.android.music/files
```

运行期会在这里留下几类文件：

- `2fa.txt`：由 `./wrapper 2fa` 写入的验证码
- `STOREFRONT_ID`：启动后保存的 storefront
- `MUSIC_TOKEN`：启动后保存的 music user token
- `mpl_db/`：Apple Music runtime 自己的数据库目录

这些文件都属于敏感运行时状态，别随手同步、别公开挂载。

## 常用环境变量

`./wrapper` 支持通过环境变量改运行方式：

| 变量 | 默认值 | 说明 |
| --- | --- | --- |
| `IMAGE_NAME` | `apple-music-wrapper` | Docker 镜像名 |
| `PLATFORM` | `linux/arm64` | 镜像平台 |
| `DATA_DIR` | `./data` | 宿主机数据目录 |
| `HOST` | `0.0.0.0` | 容器内监听地址 |
| `PUBLISH_HOST` | `127.0.0.1` | 宿主机端口绑定地址 |
| `DECRYPT_PORT` | `10020` | 解密服务端口 |
| `M3U8_PORT` | `20020` | m3u8 服务端口 |
| `ACCOUNT_PORT` | `30020` | 账号服务端口 |

例如：

```bash
PUBLISH_HOST=127.0.0.1 DATA_DIR="$PWD/runtime-data" ./wrapper start
```

## 构建方式

推荐的构建路径只有一条：

```bash
./wrapper build
```

这会调用 `docker build --platform linux/arm64 ...`，并在镜像构建阶段完成这些事：

- 下载 Android NDK `r23b`
- 用 `CMake` 交叉编译 `wrapper-core`
- 用 `aarch64-linux-gnu-gcc` 构建 chroot 启动器
- 生成最小 `tzdata`
- 修补 ELF 动态段，去掉当前运行环境不接受的 tag

如果你要改底层实现，重点看这些文件：

- [`wrapper`](wrapper)：宿主机入口脚本
- [`launch.c`](launch.c)：`chroot` 启动器
- [`server.c`](server.c)：登录、令牌、`m3u8`、解密主逻辑
- [`parser.c`](parser.c)：容器内 CLI 参数解析
- [`scripts/build_tzdata.py`](scripts/build_tzdata.py)：最小 `tzdata` 生成
- [`scripts/patch_elf.py`](scripts/patch_elf.py)：ELF patch
- [`rootfs/`](rootfs/)：运行时 root filesystem

## 边界

- 这个项目默认把敏感接口留在本机回环地址上。别轻易把它改成公网服务。
- `dev_token`、`music_token` 和解密后的数据都应该被当成敏感资产处理。
- 这不是 Apple 官方项目，也不是稳定公共接口封装。
- 请只在你有权控制和使用的账号、设备、网络环境里运行它。
