target_sources(
    joycond
    PRIVATE
        main.cpp
        phys_ctlr.cpp
        virt_ctlr.cpp
        virt_ctlr_passthrough.cpp
        virt_ctlr_combined.cpp
        virt_ctlr_pro.cpp
        epoll_mgr.cpp
        epoll_subscriber.cpp
        ctlr_detector_udev.cpp
        ctlr_mgr.cpp
    )

