use core::fmt::Arguments;
use print;

extern "C" {
    pub fn panic(msg: *const u8);
}

#[cfg(not(test))]
#[lang = "panic_fmt"]
#[no_mangle]
pub extern "C" fn panic_fmt(fmt: Arguments, file: &str, line: u32) {
    println!("Panic: {}:{}: {}", file, line, fmt);
    unsafe {
        panic("Halting\0".as_ptr());
    }

}

#[cfg(not(test))]
#[lang = "eh_personality"]
pub extern "C" fn eh_personality() {}
