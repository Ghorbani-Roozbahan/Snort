
set( FILE_LIST
    ssh.cc
    ssh.h
    ssh_config.h
    ssh_module.cc
    ssh_module.h
)

if (STATIC_INSPECTORS)
    add_library( ssh STATIC ${FILE_LIST})

else (STATIC_INSPECTORS)
    add_shared_library(ssh inspectors ${FILE_LIST})

endif (STATIC_INSPECTORS)
