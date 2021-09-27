set(
    DOODLELIB_HEADER
    core/ContainerDevice.h
    core/CoreSet.h
    core/CoreSql.h
    core/DoodleLib.h
    core/ToolsSetting.h
    core/Ue4Setting.h
    core/Util.h
    core/filesystem_extend.h
    core/observable_container.h
    core/open_file_dialog.h
    core/static_value.h
    core/tree_container.h
    Exception/Exception.h
    FileSys/FileSystem.h
    FileWarp/ImageSequence.h
    FileWarp/MayaFile.h
    FileWarp/Ue4Project.h
    FileWarp/VideoSequence.h
    Gui/base_windwos.h
    Gui/main_windwos.h
    Gui/setting_windows.h
    Gui/factory/attribute_factory_interface.h
    Gui/action/command.h
    Gui/action/command_meta.h
    Gui/action/command_tool.h
    Gui/widgets/assets_file_widgets.h
    Gui/widgets/assets_widget.h
    Gui/widgets/edit_widgets.h
    Gui/widgets/long_time_tasks_widget.h
    Gui/widgets/project_widget.h
    libWarp/BoostUuidWarp.h
    libWarp/CerealWarp.h
    libWarp/WinReg.hpp
    libWarp/boost_locale_warp.h
    libWarp/cache.hpp
    libWarp/cache_policy.hpp
    libWarp/cmrcWarp.h
    libWarp/fifo_cache_policy.hpp
    libWarp/imgui_warp.h
    libWarp/json_warp.h
    libWarp/lfu_cache_policy.hpp
    libWarp/lru_cache_policy.hpp
    libWarp/protobuf_warp.h
    libWarp/protobuf_warp_cpp.h
    libWarp/sqlppWarp.h
    libWarp/std_warp.h
    Logger/Logger.h
    Logger/LoggerTemplate.h
        Metadata/assets.h
        Metadata/assets_file.h
        Metadata/assets_path.h
        Metadata/comment.h
        Metadata/episodes.h
        Metadata/metadata.h
        Metadata/metadata_factory.h
    Metadata/Metadata_cpp.h
        Metadata/project.h
        Metadata/shot.h
    Metadata/season.h
    Metadata/time_point_wrap.h
    Metadata/user.h
    PinYin/convert.h
    rpc/RpcFileSystemClient.h
    rpc/RpcFileSystemServer.h
    rpc/RpcMetadaataServer.h
    rpc/RpcMetadataClient.h
    rpc/RpcServerHandle.h
    rpc/rpc_trans_path.h
    ScreenshotWidght/ScreenshotAction.h
    ScreenshotWidght/ScreenshotWidght.h
    threadPool/ThreadPool.h
    threadPool/long_term.h
    toolkit/toolkit.h
    DoodleLib.h
    DoodleLib_fwd.h
    DoodleLib_pch.h
    DoodleMacro.h
    doodle_app.h)
set(
    DOODLELIB_SOURCE
    core/CoreSet.cpp
    core/CoreSql.cpp
    core/DoodleLib.cpp
    core/Ue4Setting.cpp
    core/Util.cpp
    core/filesystem_extend.cpp
    core/open_file_dialog.cpp
    core/static_value.cpp
    core/tree_container.cpp
    FileSys/FileSystem.cpp
    FileWarp/ImageSequence.cpp
    FileWarp/MayaFile.cpp
    FileWarp/Ue4Project.cpp
    FileWarp/VideoSequence.cpp
    Gui/base_windwos.cpp
    Gui/main_windwos.cpp
    Gui/setting_windows.cpp
    Gui/factory/attribute_factory_interface.cpp
    Gui/action/command.cpp
    Gui/action/command_meta.cpp
    Gui/action/command_tool.cpp
    Gui/widgets/assets_file_widgets.cpp
    Gui/widgets/assets_widget.cpp
    Gui/widgets/edit_widgets.cpp
    Gui/widgets/long_time_tasks_widget.cpp
    Gui/widgets/project_widget.cpp
    libWarp/boost_serialization_warp.cpp
    libWarp/imgui_warp.cpp
    Logger/Logger.cpp
        Metadata/assets.cpp
        Metadata/assets_file.cpp
        Metadata/assets_path.cpp
        Metadata/comment.cpp
        Metadata/episodes.cpp
        Metadata/metadata.cpp
        Metadata/metadata_factory.cpp
        Metadata/project.cpp
        Metadata/shot.cpp
    Metadata/season.cpp
    Metadata/time_point_wrap.cpp
    Metadata/user.cpp
    PinYin/convert.cpp
    rpc/RpcFileSystemClient.cpp
    rpc/RpcFileSystemServer.cpp
    rpc/RpcMetadaataServer.cpp
    rpc/RpcMetadataClient.cpp
    rpc/RpcServerHandle.cpp
    rpc/rpc_trans_path.cpp
    ScreenshotWidght/ScreenshotAction.cpp
    ScreenshotWidght/ScreenshotWidght.cpp
    threadPool/long_term.cpp
    toolkit/toolkit.cpp
    DoodleLib.cpp
    doodle_app.cpp)
