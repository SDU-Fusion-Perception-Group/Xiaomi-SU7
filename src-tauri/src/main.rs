// Prevents additional console window on Windows in release, DO NOT REMOVE!!
#![cfg_attr(not(debug_assertions), windows_subsystem = "windows")]

fn main() {
    std::panic::set_hook(Box::new(|info| {
        use std::io::Write;
        if let Some(root) = std::env::var_os("LOCALAPPDATA") {
            let dir = std::path::PathBuf::from(root).join("cn.gongchuang.station");
            if std::fs::create_dir_all(&dir).is_ok() {
                if let Ok(mut log) = std::fs::File::create(dir.join("startup-error.log")) {
                    let _ = writeln!(log, "{info}\n{}", std::backtrace::Backtrace::force_capture());
                }
            }
        }
        eprintln!("{info}");
    }));
    car_ground_station_v2_lib::run()
}
