use aes::Aes128;
use block_modes::block_padding::Pkcs7;
use block_modes::{BlockMode, Cfb};
use hex::encode;
use hex_literal::hex;

type Aes128ECfb = Cfb<Aes128, Pkcs7>;

fn main() {
    let shellcode = include_bytes!("..\\..\\..\\calc.bin").as_slice();

    let key = encode("gWW8QklFyVIQfpDN");

    let iv = hex!("57504c385a78736f336b4946426a626f");

    println!("==128-bit AES CFB Mode==");
    println!("Key: {}", key);
    println!("iv: {}", encode(iv));

    let encrypted_ciphertext = enc(shellcode, &key, iv);
    println!("\nEncrypted: {}", encrypted_ciphertext);

    let decrypted_ciphertext = dec(&encrypted_ciphertext, &key, iv);
    println!("\nDecrypted: {}", decrypted_ciphertext);
}

fn enc(shellcode: &[u8], key: &String, iv: [u8; 16]) -> String {
    let key = hex::decode(key).expect("Decoding failed");

    let cipher = Aes128ECfb::new_from_slices(&key, &iv).unwrap();

    let pos = shellcode.len();

    let mut buffer = [0u8; 1280];

    buffer[..pos].copy_from_slice(shellcode);

    let ciphertext = cipher.encrypt(&mut buffer, pos).unwrap();

    hex::encode(ciphertext)
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
