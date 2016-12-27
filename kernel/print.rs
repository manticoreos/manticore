use core::fmt::{self, Write};

extern "C" {
    pub fn console_write_char(ch: u8);
}

pub static mut CONSOLE: Console = Console;
pub struct Console;

impl Write for Console {
    fn write_str(&mut self, s: &str) -> Result<(), fmt::Error> {
        for byte in s.bytes() {
            unsafe {
                console_write_char(byte);
            }
        }
        Ok(())
    }
}

/// Print to console, with a newline.
#[macro_export]
macro_rules! println {
    ($fmt:expr) => (print!(concat!($fmt, "\n")));
    ($fmt:expr, $($arg:tt)*) => (print!(concat!($fmt, "\n"), $($arg)*));
}

/// Print to console.
#[macro_export]
macro_rules! print {
    ($($arg:tt)*) => ({
        use core::fmt::Write;
        unsafe { print::CONSOLE.write_fmt(format_args!($($arg)*)).unwrap(); }
    });
}
