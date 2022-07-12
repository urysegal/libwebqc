#pragma once

#define WQC_FREE_ACCESS_TOKEN "webqc-free-access" /// This token can be used to access free services

/// List of all WQC handler options
typedef enum wqc_option_type
{
    WQC_OPTION_ACCESS_TOKEN = 1, /// Set the access token WebQC server uses to authenticate calls
} wqc_option_t;