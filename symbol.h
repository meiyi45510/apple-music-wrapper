#pragma once

#include <stdint.h>

#include "layout.h"

extern void _resolv_set_nameservers_for_net(
    unsigned netid, const char **servers, int numservers, const char *domains);

// dialog
extern union std_string *_ZNK17storeservicescore14ProtocolDialog5titleEv(void *);
extern void *_ZTVNSt6__ndk120__shared_ptr_emplaceIN17storeservicescore22ProtocolDialogResponseENS_9allocatorIS2_EEEE;
extern void _ZN17storeservicescore22ProtocolDialogResponseC1Ev(void *);
extern struct std_vector *_ZNK17storeservicescore14ProtocolDialog7buttonsEv(void *);
extern union std_string *_ZNK17storeservicescore14ProtocolButton5titleEv(void *);
extern void _ZN17storeservicescore19CredentialsResponseC1Ev(void *);
extern void _ZN17storeservicescore22ProtocolDialogResponse17setSelectedButtonERKNSt6__ndk110shared_ptrINS_14ProtocolButtonEEE(void *, struct shared_ptr *);
extern void _ZN20androidstoreservices28AndroidPresentationInterface28handleProtocolDialogResponseERKlRKNSt6__ndk110shared_ptrIN17storeservicescore22ProtocolDialogResponseEEE(void *, long *, struct shared_ptr *);

// credentials
extern uint8_t _ZNK17storeservicescore18CredentialsRequest28requiresHSA2VerificationCodeEv(void *);
extern void *_ZTVNSt6__ndk120__shared_ptr_emplaceIN17storeservicescore19CredentialsResponseENS_9allocatorIS2_EEEE;
extern void _ZN17storeservicescore19CredentialsResponse11setUserNameERKNSt6__ndk112basic_stringIcNS1_11char_traitsIcEENS1_9allocatorIcEEEE(void *, union std_string *);
extern void _ZN17storeservicescore19CredentialsResponse11setPasswordERKNSt6__ndk112basic_stringIcNS1_11char_traitsIcEENS1_9allocatorIcEEEE(void *, union std_string *);
extern void _ZN17storeservicescore19CredentialsResponse15setResponseTypeENS0_12ResponseTypeE(void *, int response_type);
extern void _ZN20androidstoreservices28AndroidPresentationInterface25handleCredentialsResponseERKNSt6__ndk110shared_ptrIN17storeservicescore19CredentialsResponseEEE(void *, struct shared_ptr *);

// fairplay
extern void _ZN21SVFootHillSessionCtrl16getPersistentKeyERKNSt6__ndk112basic_stringIcNS0_11char_traitsIcEENS0_9allocatorIcEEEES8_S8_S8_S8_S8_S8_S8_(struct shared_ptr *, void *, union std_string *, union std_string *, union std_string *, union std_string *, union std_string *, union std_string *, union std_string *, union std_string *);
static inline void
_ZN21SVFootHillSessionCtrl16getPersistentKeyERKNSt6__ndk112basic_stringIcNS0_11char_traitsIcEENS0_9allocatorIcEEEES8_S8_S8_S8_S8_S8_S8_ASM(
    struct shared_ptr *persistent_key, void *instance,
    union std_string *adam_id, union std_string *prefetch_adam_id,
    union std_string *key_uri, union std_string *key_format,
    union std_string *key_format_ver, union std_string *server_uri,
    union std_string *protocol_type, union std_string *fps_cert) {
    asm volatile(
        "mov x8, %0\n"
        "mov x0, %1\n"
        "mov x1, %2\n"
        "mov x2, %3\n"
        "mov x3, %4\n"
        "mov x4, %5\n"
        "mov x5, %6\n"
        "mov x6, %7\n"
        "mov x7, %8\n"
        "sub sp, sp, #0x10\n"
        "mov x9, %9\n"
        "str x9, [sp]\n"
        "bl _ZN21SVFootHillSessionCtrl16getPersistentKeyERKNSt6__ndk112basic_stringIcNS0_11char_traitsIcEENS0_9allocatorIcEEEES8_S8_S8_S8_S8_S8_S8_\n"
        "add sp, sp, #0x10\n"
        :
        : "r" (persistent_key), "r" (instance), "r" (adam_id),
          "r" (prefetch_adam_id), "r" (key_uri), "r" (key_format),
          "r" (key_format_ver), "r" (server_uri), "r" (protocol_type),
          "r" (fps_cert)
        : "x8", "x0", "x1", "x2", "x3", "x4", "x5", "x6", "x7", "x9", "memory", "lr"
    );
}
extern void _ZN21SVFootHillSessionCtrl14decryptContextERKNSt6__ndk112basic_stringIcNS0_11char_traitsIcEENS0_9allocatorIcEEEERKN11SVDecryptor15SVDecryptorTypeERKb(struct shared_ptr *, void *, void *);
static inline void
_ZN21SVFootHillSessionCtrl14decryptContextERKNSt6__ndk112basic_stringIcNS0_11char_traitsIcEENS0_9allocatorIcEEEERKN11SVDecryptor15SVDecryptorTypeERKbASM(
    struct shared_ptr *decrypted_context, void *instance, void *context) {
    asm volatile(
        "mov x8, %0\n"
        "mov x0, %1\n"
        "mov x1, %2\n"
        "bl _ZN21SVFootHillSessionCtrl14decryptContextERKNSt6__ndk112basic_stringIcNS0_11char_traitsIcEENS0_9allocatorIcEEEERKN11SVDecryptor15SVDecryptorTypeERKb\n"
        :
        : "r" (decrypted_context), "r" (instance), "r" (context)
        : "x8", "x0", "x1", "memory", "lr"
    );
}
extern void **_ZNK18SVFootHillPContext9kdContextEv(void *);
extern long NfcRKVnxuKZy04KWbdFu71Ou(void *, uint32_t, void *, void *, size_t);
extern void *_ZN21SVFootHillSessionCtrl8instanceEv();
static inline void *get_foot_hill_instance(void) {
    return _ZN21SVFootHillSessionCtrl8instanceEv();
}
static const char *const fairplay_cert = "MIIEzjCCA7agAwIBAgIIAXAVjHFZDjgwDQYJKoZIhvcNAQEFBQAwfzELMAkGA1UEBhMCVVMxEzARBgNVBAoMCkFwcGxlIEluYy4xJjAkBgNVBAsMHUFwcGxlIENlcnRpZmljYXRpb24gQXV0aG9yaXR5MTMwMQYDVQQDDCpBcHBsZSBLZXkgU2VydmljZXMgQ2VydGlmaWNhdGlvbiBBdXRob3JpdHkwHhcNMTIwNzI1MTgwMjU4WhcNMTQwNzI2MTgwMjU4WjAwMQswCQYDVQQGEwJVUzESMBAGA1UECgwJQXBwbGUgSW5jMQ0wCwYDVQQDDARGUFMxMIGfMA0GCSqGSIb3DQEBAQUAA4GNADCBiQKBgQCqZ9IbMt0J0dTKQN4cUlfeQRY9bcnbnP95HFv9A16Yayh4xQzRLAQqVSmisZtBK2/nawZcDmcs+XapBojRb+jDM4Dzk6/Ygdqo8LoA+BE1zipVyalGLj8Y86hTC9QHX8i05oWNCDIlmabjjWvFBoEOk+ezOAPg8c0SET38x5u+TwIDAQABo4ICHzCCAhswHQYDVR0OBBYEFPP6sfTWpOQ5Sguf5W3Y0oibbEc3MAwGA1UdEwEB/wQCMAAwHwYDVR0jBBgwFoAUY+RHVMuFcVlGLIOszEQxZGcDLL4wgeIGA1UdIASB2jCB1zCB1AYJKoZIhvdjZAUBMIHGMIHDBggrBgEFBQcCAjCBtgyBs1JlbGlhbmNlIG9uIHRoaXMgY2VydGlmaWNhdGUgYnkgYW55IHBhcnR5IGFzc3VtZXMgYWNjZXB0YW5jZSBvZiB0aGUgdGhlbiBhcHBsaWNhYmxlIHN0YW5kYXJkIHRlcm1zIGFuZCBjb25kaXRpb25zIG9mIHVzZSwgY2VydGlmaWNhdGUgcG9saWN5IGFuZCBjZXJ0aWZpY2F0aW9uIHByYWN0aWNlIHN0YXRlbWVudHMuMDUGA1UdHwQuMCwwKqAooCaGJGh0dHA6Ly9jcmwuYXBwbGUuY29tL2tleXNlcnZpY2VzLmNybDAOBgNVHQ8BAf8EBAMCBSAwFAYLKoZIhvdjZAYNAQUBAf8EAgUAMBsGCyqGSIb3Y2QGDQEGAQH/BAkBAAAAAQAAAAEwKQYLKoZIhvdjZAYNAQMBAf8EFwF+bjsY57ASVFmeehD2bdu6HLGBxeC2MEEGCyqGSIb3Y2QGDQEEAQH/BC8BHrKviHJf/Se/ibc7T0/55Bt1GePzaYBVfgF3ZiNuV93z8P3qsawAqAXzzh9o5DANBgkqhkiG9w0BAQUFAAOCAQEAVGyCtuLYcYb/aPijBCtaemxuV0IokXJn3EgmwYHZynaR6HZmeGRUp9p3f8EXu6XPSekKCCQi+a86hXX9RfnGEjRdvtP+jts5MDSKuUIoaqce8cLX2dpUOZXdf3lR0IQM0kXHb5boNGBsmbTLVifqeMsexfZryGw2hE/4WDOJdGQm1gMJZU4jP1b/HSLNIUhHWAaMeWtcJTPRBucR4urAtvvtOWD88mriZNHG+veYw55b+qA36PSqDPMbku9xTY7fsMa6mxIRmwULQgi8nOk1wNhw3ZO0qUKtaCO3gSqWdloecxpxUQSZCSW7tWPkpXXwDZqegUkij9xMFS1pr37RIjCCBVAwggQ4oAMCAQICEEVKuaGraq1Cp4z6TFOeVfUwDQYJKoZIhvcNAQELBQAwUDEsMCoGA1UEAwwjQXBwbGUgRlAgU2VydmljZSBFbmFibGUgUlNBIENBIC0gRzExEzARBgNVBAoMCkFwcGxlIEluYy4xCzAJBgNVBAYTAlVTMB4XDTIwMDQwNzIwMjY0NFoXDTIyMDQwNzIwMjY0NFowWjEhMB8GA1UEAwwYZnBzMjA0OC5pdHVuZXMuYXBwbGUuY29tMRMwEQYDVQQLDApBcHBsZSBJbmMuMRMwEQYDVQQKDApBcHBsZSBJbmMuMQswCQYDVQQGEwJVUzCCASIwDQYJKoZIhvcNAQEBBQADggEPADCCAQoCggEBAJNoUHuTRLafofQgIRgGa2TFIf+bsFDMjs+y3Ep1xCzFLE4QbnwG6OG0duKUl5IoGUsouzZk9iGsXz5k3ESLOWKz2BFrDTvGrzAcuLpH66jJHGsk/l+ZzsDOJaoQ22pu0JvzYzW8/yEKvpE6JF/2dsC6V9RDTri3VWFxrl5uh8czzncoEQoRcQsSatHzs4tw/QdHFtBIigqxqr4R7XiCaHbsQmqbP9h7oxRs/6W/DDA2BgkuFY1ocX/8dTjmH6szKPfGt3KaYCwy3fuRC+FibTyohtvmlXsYhm7AUzorwWIwN/MbiFQ0OHHtDomIy71wDcTNMnY0jZYtGmIlJETAgYcCAwEAAaOCAhowggIWMAwGA1UdEwEB/wQCMAAwHwYDVR0jBBgwFoAUrI/yBkpV623/IeMrXzs8fC7VkZkwRQYIKwYBBQUHAQEEOTA3MDUGCCsGAQUFBzABhilodHRwOi8vb2NzcC5hcHBsZS5jb20vb2NzcDAzLWZwc3J2cnNhZzEwMzCBwwYDVR0gBIG7MIG4MIG1BgkqhkiG92NkBQEwgacwgaQGCCsGAQUFBwICMIGXDIGUUmVsaWFuY2Ugb24gdGhpcyBjZXJ0aWZpY2F0ZSBieSBhbnkgcGFydHkgYXNzdW1lcyBhY2NlcHRhbmNlIG9mIGFueSBhcHBsaWNhYmxlIHRlcm1zIGFuZCBjb25kaXRpb25zIG9mIHVzZSBhbmQvb3IgY2VydGlmaWNhdGlvbiBwcmFjdGljZSBzdGF0ZW1lbnRzLjAdBgNVHQ4EFgQU2RpCSSHFXeoZQQWxbwJuRZ9RrIEwDgYDVR0PAQH/BAQDAgUgMBQGCyqGSIb3Y2QGDQEFAQH/BAIFADAjBgsqhkiG92NkBg0BBgEB/wQRAQAAAAMAAAABAAAAAgAAAAMwOQYLKoZIhvdjZAYNAQMBAf8EJwG+pUeWbeZBUI0PikyFwSggL5dHaeugSDoQKwcP28csLuh5wplpATAzBgsqhkiG92NkBg0BBAEB/wQhAfl9TGjP/UY9TyQzYsn8sX9ZvHChok9QrrUhtAyWR1yCMA0GCSqGSIb3DQEBCwUAA4IBAQBNMzZ6llQ0laLXsrmyVieuoW9+pHeAaDJ7cBiQLjM3ZdIO3Gq5dkbWYYYwJwymdxZ74WGZMuVv3ueJKcxG1jAhCRhr0lb6QaPaQQSNW+xnoesb3CLA0RzrcgBp/9WFZNdttJOSyC93lQmiE0r5RqPpe/IWUzwoZxri8qnsghVFxCBEcMB+U4PJR8WeAkPrji8po2JLYurvgNRhGkDKcAFPuGEpXdF86hPts+07zazsP0fBjBSVgP3jqb8G31w5W+O+wBW0B9uCf3s0vXU4LuJTAywws2ImZ7O/AaY/uXWOyIUMUKPgL1/QJieB7pBoENIJ2CeJS2M3iv00ssmCmTEJ";

// playback
extern void _ZN22SVPlaybackLeaseManagerC2ERKNSt6__ndk18functionIFvRKiEEERKNS1_IFvRKNS0_10shared_ptrIN17storeservicescore19StoreErrorConditionEEEEEE(void *, void *, void *);
extern void _ZN22SVPlaybackLeaseManager25refreshLeaseAutomaticallyERKb(void *, uint8_t *);
extern void _ZN22SVPlaybackLeaseManager12requestLeaseERKb(void *, uint8_t *);

extern void _ZN22SVPlaybackLeaseManager12requestAssetERKmRKNSt6__ndk16vectorINS2_12basic_stringIcNS2_11char_traitsIcEENS2_9allocatorIcEEEENS7_IS9_EEEERKb(void *, void *, unsigned long *, struct std_vector *, uint8_t *);
static inline void
_ZN22SVPlaybackLeaseManager12requestAssetERKmRKNSt6__ndk16vectorINS2_12basic_stringIcNS2_11char_traitsIcEENS2_9allocatorIcEEEENS7_IS9_EEEERKbASM(
    struct shared_ptr *result, void *lease_manager_obj, unsigned long *adam,
    struct std_vector *hls_param, uint8_t *z0) {
    asm volatile(
        "mov x8, %0\n"
        "mov x0, %1\n"
        "mov x1, %2\n"
        "mov x2, %3\n"
        "mov x3, %4\n"
        "bl _ZN22SVPlaybackLeaseManager12requestAssetERKmRKNSt6__ndk16vectorINS2_12basic_stringIcNS2_11char_traitsIcEENS2_9allocatorIcEEEENS7_IS9_EEEERKb\n"
        :
        : "r" (result), "r" (lease_manager_obj), "r" (adam), "r" (hls_param), "r" (z0)
        : "x8", "x0", "x1", "x2", "x3", "lr"
    );
}
extern int _ZNK23SVPlaybackAssetResponse13hasValidAssetEv(void *);
extern struct shared_ptr *_ZNK23SVPlaybackAssetResponse13playbackAssetEv(void *);
extern void _ZNK17storeservicescore13PlaybackAsset9URLStringEv(void *, uint8_t *);
static inline void _ZNK17storeservicescore13PlaybackAsset9URLStringEvASM(void *url, void *playback_asset) {
    asm volatile(
        "mov x8, %0\n"
        "mov x0, %1\n"
        "bl _ZNK17storeservicescore13PlaybackAsset9URLStringEv\n"
        :
        : "r" (url), "r" (playback_asset)
        : "x8", "x0", "lr"
    );
}

// context
extern void _ZNSt6__ndk110shared_ptrIN17storeservicescore14RequestContextEE11make_sharedIJRNS_12basic_stringIcNS_11char_traitsIcEENS_9allocatorIcEEEEEEES3_DpOT_(struct shared_ptr *, union std_string *);
static inline void
_ZNSt6__ndk110shared_ptrIN17storeservicescore14RequestContextEE11make_sharedIJRNS_12basic_stringIcNS_11char_traitsIcEENS_9allocatorIcEEEEEEES3_DpOT_ASM(
    struct shared_ptr *request_context_ptr, union std_string *str) {
    asm volatile(
        "mov x8, %0\n"
        "mov x0, %1\n"
        "bl _ZNSt6__ndk110shared_ptrIN17storeservicescore14RequestContextEE11make_sharedIJRNS_12basic_stringIcNS_11char_traitsIcEENS_9allocatorIcEEEEEEES3_DpOT_\n"
        :
        : "r" (request_context_ptr), "r" (str)
        : "x8", "x0", "lr"
    );
}
static inline struct shared_ptr *new_request_context(const char *str) {
    struct shared_ptr *request_context_ptr =
        (struct shared_ptr *)malloc(sizeof(struct shared_ptr));
    union std_string str_buf = new_std_string(str);

    _ZNSt6__ndk110shared_ptrIN17storeservicescore14RequestContextEE11make_sharedIJRNS_12basic_stringIcNS_11char_traitsIcEENS_9allocatorIcEEEEEEES3_DpOT_ASM(
        request_context_ptr, &str_buf);
    return request_context_ptr;
}

extern void *_ZTVNSt6__ndk120__shared_ptr_emplaceIN17storeservicescore20RequestContextConfigENS_9allocatorIS2_EEEE;
extern void _ZN17storeservicescore20RequestContextConfigC2Ev(void *);
static inline struct shared_ptr *new_request_context_config(void) {
    uint8_t *ptr = (uint8_t *)malloc(480);
    *(void **)(ptr) =
        &_ZTVNSt6__ndk120__shared_ptr_emplaceIN17storeservicescore20RequestContextConfigENS_9allocatorIS2_EEEE +
        2;

    struct shared_ptr *request_context_config =
        (struct shared_ptr *)malloc(sizeof(struct shared_ptr));
    request_context_config->obj = ptr + 32;
    request_context_config->ctrl_blk = ptr;

    _ZN17storeservicescore20RequestContextConfigC2Ev(request_context_config->obj);
    return request_context_config;
}

extern void _ZN17storeservicescore20RequestContextConfig20setBaseDirectoryPathERKNSt6__ndk112basic_stringIcNS1_11char_traitsIcEENS1_9allocatorIcEEEE(void *, union std_string *);
extern void _ZN17storeservicescore20RequestContextConfig19setClientIdentifierERKNSt6__ndk112basic_stringIcNS1_11char_traitsIcEENS1_9allocatorIcEEEE(void *, union std_string *);
extern void _ZN17storeservicescore20RequestContextConfig20setVersionIdentifierERKNSt6__ndk112basic_stringIcNS1_11char_traitsIcEENS1_9allocatorIcEEEE(void *, union std_string *);
extern void _ZN17storeservicescore20RequestContextConfig21setPlatformIdentifierERKNSt6__ndk112basic_stringIcNS1_11char_traitsIcEENS1_9allocatorIcEEEE(void *, union std_string *);
extern void _ZN17storeservicescore20RequestContextConfig17setProductVersionERKNSt6__ndk112basic_stringIcNS1_11char_traitsIcEENS1_9allocatorIcEEEE(void *, union std_string *);
extern void _ZN17storeservicescore20RequestContextConfig14setDeviceModelERKNSt6__ndk112basic_stringIcNS1_11char_traitsIcEENS1_9allocatorIcEEEE(void *, union std_string *);
extern void _ZN17storeservicescore20RequestContextConfig15setBuildVersionERKNSt6__ndk112basic_stringIcNS1_11char_traitsIcEENS1_9allocatorIcEEEE(void *, union std_string *);
extern void _ZN17storeservicescore20RequestContextConfig19setLocaleIdentifierERKNSt6__ndk112basic_stringIcNS1_11char_traitsIcEENS1_9allocatorIcEEEE(void *, union std_string *);
extern void _ZN17storeservicescore20RequestContextConfig21setLanguageIdentifierERKNSt6__ndk112basic_stringIcNS1_11char_traitsIcEENS1_9allocatorIcEEEE(void *, union std_string *);
extern void _ZN17storeservicescore20RequestContextConfig24setFairPlayDirectoryPathERKNSt6__ndk112basic_stringIcNS1_11char_traitsIcEENS1_9allocatorIcEEEE(void *, union std_string *);

extern void _ZN21RequestContextManager9configureERKNSt6__ndk110shared_ptrIN17storeservicescore14RequestContextEEE(struct shared_ptr *);
static inline void configure_request_context(
    struct shared_ptr *request_context_ptr) {
    _ZN21RequestContextManager9configureERKNSt6__ndk110shared_ptrIN17storeservicescore14RequestContextEEE(request_context_ptr);
}

extern void _ZN17storeservicescore14RequestContext4initERKNSt6__ndk110shared_ptrINS_20RequestContextConfigEEE(void *, void *, struct shared_ptr *);
static inline void
_ZN17storeservicescore14RequestContext4initERKNSt6__ndk110shared_ptrINS_20RequestContextConfigEEEASM(
    uint8_t *buf, struct shared_ptr *request_context_ptr,
    struct shared_ptr *request_context_config) {
    asm volatile(
        "mov x8, %0\n"
        "mov x0, %1\n"
        "mov x1, %2\n"
        "bl _ZN17storeservicescore14RequestContext4initERKNSt6__ndk110shared_ptrINS_20RequestContextConfigEEE\n"
        :
        : "r" (buf), "r" (request_context_ptr), "r" (request_context_config)
        : "x8", "x0", "x1", "lr"
    );
}
static inline void init_request_context(
    uint8_t *buf, struct shared_ptr *request_context_ptr,
    struct shared_ptr *request_context_config) {
    _ZN17storeservicescore14RequestContext4initERKNSt6__ndk110shared_ptrINS_20RequestContextConfigEEEASM(
        buf, request_context_ptr->obj, request_context_config);
}

extern void _ZN17storeservicescore14RequestContext24setFairPlayDirectoryPathERKNSt6__ndk112basic_stringIcNS1_11char_traitsIcEENS1_9allocatorIcEEEE(void *, union std_string *);

static inline void set_fair_play_directory_path(
    struct shared_ptr *request_context_ptr, const char *str) {
    union std_string str_buf = new_std_string(str);

    _ZN17storeservicescore14RequestContext24setFairPlayDirectoryPathERKNSt6__ndk112basic_stringIcNS1_11char_traitsIcEENS1_9allocatorIcEEEE(
        request_context_ptr->obj, &str_buf);
}

// ui bridge
extern void _ZNSt6__ndk110shared_ptrIN20androidstoreservices28AndroidPresentationInterfaceEE11make_sharedIJEEES3_DpOT_(struct shared_ptr *);
static inline void _ZNSt6__ndk110shared_ptrIN20androidstoreservices28AndroidPresentationInterfaceEE11make_sharedIJEEES3_DpOT_ASM(struct shared_ptr *presentation_interface_ptr) {
    asm volatile(
        "mov x8, %0\n"
        "bl _ZNSt6__ndk110shared_ptrIN20androidstoreservices28AndroidPresentationInterfaceEE11make_sharedIJEEES3_DpOT_\n"
        :
        : "r" (presentation_interface_ptr)
        : "x8", "x0", "lr"
    );
}
extern void _ZN20androidstoreservices28AndroidPresentationInterface16setDialogHandlerEPFvlNSt6__ndk110shared_ptrIN17storeservicescore14ProtocolDialogEEENS2_INS_36AndroidProtocolDialogResponseHandlerEEEE(void *, void (*)(long, struct shared_ptr *, struct shared_ptr *));
extern void _ZN20androidstoreservices28AndroidPresentationInterface21setCredentialsHandlerEPFvNSt6__ndk110shared_ptrIN17storeservicescore18CredentialsRequestEEENS2_INS_33AndroidCredentialsResponseHandlerEEEE(void *, void (*)(struct shared_ptr *, struct shared_ptr *));
static inline void init_presentation_interface(
    struct shared_ptr *presentation_interface,
    void (*dialog_handler)(long, struct shared_ptr *, struct shared_ptr *),
    void (*credentials_handler)(struct shared_ptr *, struct shared_ptr *)) {
    _ZNSt6__ndk110shared_ptrIN20androidstoreservices28AndroidPresentationInterfaceEE11make_sharedIJEEES3_DpOT_ASM(
        presentation_interface);
    _ZN20androidstoreservices28AndroidPresentationInterface16setDialogHandlerEPFvlNSt6__ndk110shared_ptrIN17storeservicescore14ProtocolDialogEEENS2_INS_36AndroidProtocolDialogResponseHandlerEEEE(
        presentation_interface->obj, dialog_handler);
    _ZN20androidstoreservices28AndroidPresentationInterface21setCredentialsHandlerEPFvNSt6__ndk110shared_ptrIN17storeservicescore18CredentialsRequestEEENS2_INS_33AndroidCredentialsResponseHandlerEEEE(
        presentation_interface->obj, credentials_handler);
}

extern void _ZN17storeservicescore14RequestContext24setPresentationInterfaceERKNSt6__ndk110shared_ptrINS_21PresentationInterfaceEEE(void *, struct shared_ptr *);
static inline void set_presentation_interface(
    struct shared_ptr *request_context_ptr,
    struct shared_ptr *presentation_interface_ptr) {
    _ZN17storeservicescore14RequestContext24setPresentationInterfaceERKNSt6__ndk110shared_ptrINS_21PresentationInterfaceEEE(
        request_context_ptr->obj, presentation_interface_ptr);
}

// login
extern void _ZNSt6__ndk110shared_ptrIN17storeservicescore16AuthenticateFlowEE11make_sharedIJRNS0_INS1_14RequestContextEEEEEES3_DpOT_(struct shared_ptr *, struct shared_ptr *);
static inline void
_ZNSt6__ndk110shared_ptrIN17storeservicescore16AuthenticateFlowEE11make_sharedIJRNS0_INS1_14RequestContextEEEEEES3_DpOT_ASM(
    struct shared_ptr *flow, struct shared_ptr *request_context_ptr) {
    asm volatile(
        "mov x8, %0\n"
        "mov x0, %1\n"
        "bl _ZNSt6__ndk110shared_ptrIN17storeservicescore16AuthenticateFlowEE11make_sharedIJRNS0_INS1_14RequestContextEEEEEES3_DpOT_\n"
        :
        : "r" (flow), "r" (request_context_ptr)
        : "x8", "x0", "lr"
    );
}
extern void _ZN17storeservicescore16AuthenticateFlow3runEv(void *);
extern struct shared_ptr *_ZNK17storeservicescore16AuthenticateFlow8responseEv(void *);
extern int _ZNK17storeservicescore20AuthenticateResponse12responseTypeEv(void *);
extern void _ZN14FootHillConfig6configERKNSt6__ndk112basic_stringIcNS0_11char_traitsIcEENS0_9allocatorIcEEEE(union std_string *);
extern void _ZN17storeservicescore10DeviceGUID8instanceEv(struct shared_ptr *);
static inline void _ZN17storeservicescore10DeviceGUID8instanceEvASM(
    struct shared_ptr *guid_handle) {
    asm volatile(
        "mov x8, %0\n"
        "bl _ZN17storeservicescore10DeviceGUID8instanceEv\n"
        :
        : "r" (guid_handle)
        : "x8", "x0", "lr"
    );
}
extern void _ZN17storeservicescore10DeviceGUID9configureERKNSt6__ndk112basic_stringIcNS1_11char_traitsIcEENS1_9allocatorIcEEEES9_RKjRKb(void *, void *, union std_string *, union std_string *, unsigned int *, uint8_t *);
static inline void
_ZN17storeservicescore10DeviceGUID9configureERKNSt6__ndk112basic_stringIcNS1_11char_traitsIcEENS1_9allocatorIcEEEES9_RKjRKbASM(
    void *return_buffer, void *guid_handle, union std_string *config_1,
    union std_string *config_2, unsigned int *config_3, uint8_t *config_4) {
    asm volatile(
        "mov x8, %0\n"
        "mov x0, %1\n"
        "mov x1, %2\n"
        "mov x2, %3\n"
        "mov x3, %4\n"
        "mov x4, %5\n"
        "bl _ZN17storeservicescore10DeviceGUID9configureERKNSt6__ndk112basic_stringIcNS1_11char_traitsIcEENS1_9allocatorIcEEEES9_RKjRKb\n"
        :
        : "r" (return_buffer), "r" (guid_handle), "r" (config_1),
          "r" (config_2), "r" (config_3), "r" (config_4)
        : "x8", "x0", "x1", "x2", "x3", "x4", "lr"
    );
}

// tokens
extern union std_string *_ZNK17storeservicescore14RequestContext20storeFrontIdentifierERKNSt6__ndk110shared_ptrINS_6URLBagEEE(void *, void *, struct shared_ptr *);
static inline void
_ZNK17storeservicescore14RequestContext20storeFrontIdentifierERKNSt6__ndk110shared_ptrINS_6URLBagEEEASM(
    void *region_out, void *request_context_obj, struct shared_ptr *url_bag) {
    asm volatile(
        "mov x8, %0\n"
        "mov x0, %1\n"
        "mov x1, %2\n"
        "bl _ZNK17storeservicescore14RequestContext20storeFrontIdentifierERKNSt6__ndk110shared_ptrINS_6URLBagEEE\n"
        :
        : "r" (region_out), "r" (request_context_obj), "r" (url_bag)
        : "x8", "x0", "x1", "lr"
    );
}
extern void *_ZTVNSt6__ndk120__shared_ptr_emplaceIN13mediaplatform11HTTPMessageENS_9allocatorIS2_EEEE;
extern void *_ZN13mediaplatform11HTTPMessageC2ENSt6__ndk112basic_stringIcNS1_11char_traitsIcEENS1_9allocatorIcEEEES7_(void *, union std_string *, union std_string *);
extern void _ZN13mediaplatform11HTTPMessage9setHeaderERKNSt6__ndk112basic_stringIcNS1_11char_traitsIcEENS1_9allocatorIcEEEES9_(void *, union std_string *, union std_string *);
extern void _ZN13mediaplatform11HTTPMessage11setBodyDataEPcm(void *, char *, u_long);
extern void *_ZN17storeservicescore10DeviceGUID4guidEv(void *p1, void *p2);
static inline void _ZN17storeservicescore10DeviceGUID4guidEvASM(
    void *ret_buffer, void *guid_instance) {
    asm volatile(
        "mov x8, %0\n"
        "mov x0, %1\n"
        "bl _ZN17storeservicescore10DeviceGUID4guidEv\n"
        :
        : "r" (ret_buffer), "r" (guid_instance)
        : "x8", "x0", "lr"
    );
}
extern char *_ZNK13mediaplatform4Data5bytesEv(void *);
extern void *_ZN17storeservicescore10URLRequestC2ERKNSt6__ndk110shared_ptrIN13mediaplatform11HTTPMessageEEERKNS2_INS_14RequestContextEEE(void *, struct shared_ptr *, struct shared_ptr *);
extern void *_ZN17storeservicescore10URLRequest19setRequestParameterERKNSt6__ndk112basic_stringIcNS1_11char_traitsIcEENS1_9allocatorIcEEEES9_(void *, union std_string *, union std_string *);
extern void *_ZN17storeservicescore10URLRequest3runEv(void *);
extern struct shared_ptr *_ZNK17storeservicescore10URLRequest5errorEv(void *);
extern struct shared_ptr *_ZNK17storeservicescore10URLRequest8responseEv(void *);
extern struct shared_ptr *_ZNK17storeservicescore11URLResponse18underlyingResponseEv(void *);

// offline
extern void *_ZN17storeservicescore15PurchaseRequestC2ERKNSt6__ndk110shared_ptrINS_14RequestContextEEE(void *, struct shared_ptr *);
extern void *_ZN17storeservicescore15PurchaseRequest23setProcessDialogActionsEb(void *, int);
extern void *_ZN17storeservicescore15PurchaseRequest12setURLBagKeyERKNSt6__ndk112basic_stringIcNS1_11char_traitsIcEENS1_9allocatorIcEEEE(void *, union std_string *);
extern void *_ZN17storeservicescore15PurchaseRequest16setBuyParametersERKNSt6__ndk112basic_stringIcNS1_11char_traitsIcEENS1_9allocatorIcEEEE(void *, union std_string *);
extern void *_ZN17storeservicescore15PurchaseRequest3runEv(void *);
extern struct shared_ptr *_ZNK17storeservicescore15PurchaseRequest8responseEv(void *);
extern struct shared_ptr *_ZN17storeservicescore16PurchaseResponse5errorEv(void *);
extern struct std_vector _ZNK17storeservicescore16PurchaseResponse5itemsEv(void *);
extern struct std_vector _ZNK17storeservicescore12PurchaseItem6assetsEv(void *);
extern union std_string *_ZNK17storeservicescore13PurchaseAsset3URLEv(void *, void *);
static inline void _ZNK17storeservicescore13PurchaseAsset3URLEvASM(
    void *buf, void *asset_obj) {
    asm volatile(
        "mov x8, %0\n"
        "mov x0, %1\n"
        "bl _ZNK17storeservicescore13PurchaseAsset3URLEv\n"
        :
        : "r" (buf), "r" (asset_obj)
        : "x8", "x0", "lr", "memory"
    );
}

extern void *_ZN17storeservicescore14RequestContext8fairPlayEv(void *, void *);
static inline void _ZN17storeservicescore14RequestContext8fairPlayEvASM(
    void *buf, void *request_context_obj) {
    asm volatile(
        "mov x8, %0\n"
        "mov x0, %1\n"
        "bl _ZN17storeservicescore14RequestContext8fairPlayEv\n"
        :
        : "r" (buf), "r" (request_context_obj)
        : "x8", "x0", "lr", "memory"
    );
}

extern struct std_vector _ZN17storeservicescore8FairPlay21getSubscriptionStatusEv(void *);
