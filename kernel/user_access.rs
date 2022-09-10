// User space memory access routines.

extern "C" {
    pub fn memcpy_to_user(dest: *mut u8, src: *const u8, len: usize) -> *mut u8;
}
