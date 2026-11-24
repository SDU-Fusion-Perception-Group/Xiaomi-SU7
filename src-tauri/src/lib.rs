use tauri::WebviewWindowBuilder;

fn record_startup_error(error: &dyn std::fmt::Debug) {
    let text = format!("Ground station startup failed: {error:#?}\n");
    let dir = std::env::var_os("LOCALAPPDATA")
        .map(std::path::PathBuf::from)
        .unwrap_or_else(std::env::temp_dir)
        .join("cn.gongchuang.station");
    if std::fs::create_dir_all(&dir)
        .and_then(|()| std::fs::write(dir.join("startup-error.log"), &text))
        .is_err()
    {
        let _ = std::fs::write(std::env::temp_dir().join("station-startup-error.log"), &text);
    }
    eprint!("{text}");
}

#[cfg_attr(mobile, tauri::mobile_entry_point)]
pub fn run() {
    let result = tauri::Builder::default()
        .setup(|app| {
            let config = &app.config().app.windows[0];
            if let Err(error) = WebviewWindowBuilder::from_config(app, config)
                .and_then(|builder| builder.build())
            {
                // The portable EXE can use a cache beside itself when Windows
                // refuses to initialize WebView2 in the default user profile.
                #[cfg(windows)]
                if matches!(error, tauri::Error::Io(_) | tauri::Error::Runtime(_)) {
                    let fallback = std::env::current_exe()
                        .map(|mut path| {
                            path.set_extension("webview2");
                            path
                        })
                        .map_err(tauri::Error::from)
                        .and_then(|path| {
                            WebviewWindowBuilder::from_config(app, config)?
                                .data_directory(path)
                                .build()
                        });
                    match fallback {
                        Ok(_) => return Ok(()),
                        Err(fallback_error) => record_startup_error(&(error, fallback_error)),
                    }
                } else {
                    record_startup_error(&error);
                }
                #[cfg(not(windows))]
                record_startup_error(&error);
                app.handle().exit(1);
            }
            Ok(())
        })
        .run(tauri::generate_context!());
    if let Err(error) = result {
        record_startup_error(&error);
        std::process::exit(1);
    }
}
