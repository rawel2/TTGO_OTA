# Edit following two lines to set component requirements (see docs)
set(COMPONENT_REQUIRES )
set(COMPONENT_PRIV_REQUIRES )

set(COMPONENT_SRCS "main.c" "mk_fota_https.c")
set(COMPONENT_ADD_INCLUDEDIRS ".")
set(COMPONENT_EMBED_TXTFILES "../server_certs/ca_cert.cer")


register_component()
