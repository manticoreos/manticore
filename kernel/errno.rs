use core::result;

pub const ENOMEM: i32 = 12;
pub const EINVAL: i32 = 22;
pub const ENOSYS: i32 = 38;

#[derive(Clone, Copy, Debug, PartialEq)]
pub struct Error(i32);

pub type Result<T> = result::Result<T, Error>;

impl Error {
    pub fn new(err: i32) -> Error {
        Error(err)
    }

    pub fn errno(&self) -> i32 {
        self.0
    }
}
