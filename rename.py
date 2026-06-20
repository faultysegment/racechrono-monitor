import os

files_to_update = [
    "src/View.h",
    "src/Device_Native/App.h",
    "src/Device_Native/Screens/MonitorScreen.h",
    "src/Device_Native/Screens/DualMonitorScreen.h",
    "src/Device_Native/Screens/DisconnectedMsgScreen.h",
    "src/Device_Native/Screens/ConfigMonitorScreen.h",
    "src/Device_T_Embed_CC1101/Screens/MonitorScreen.h",
    "src/Device_T_Embed_CC1101/Screens/DualMonitorScreen.h",
    "src/Device_T_Embed_CC1101/Screens/DisconnectedMsgScreen.h",
    "src/Device_T_Embed_CC1101/Screens/ConfigMonitorScreen.h",
    "src/Device_T_Embed_CC1101/App.h",
    "src/AppLogic.h",
    "src/Device_All/Screens/IScreen.h",
    "test/test_view/test_view.cpp",
    "test/test_appstate/test_appstate.cpp",
    "test/test_applogic/test_applogic.cpp",
    "AGENTS.md"
]

for file_path in files_to_update:
    if os.path.exists(file_path):
        with open(file_path, "r", encoding="utf-8") as f:
            content = f.read()
        
        content = content.replace("UIState", "AppState")
        content = content.replace("UIState.h", "AppState.h")
        content = content.replace("uistate", "appstate")
        
        with open(file_path, "w", encoding="utf-8") as f:
            f.write(content)
        print(f"Updated {file_path}")
    else:
        print(f"Not found {file_path}")
