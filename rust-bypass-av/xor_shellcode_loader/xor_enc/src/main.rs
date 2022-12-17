fn main() {
    let shellcode = include_bytes!("..\\..\\..\\calc.bin").as_slice();
    let key = "Vpe0YiGeYjKj5kMAsF0PjMEurwsMPZZY";

    let encrypted = xor_encrypt(&shellcode, &key.as_bytes());
    println!("Encrypted: {}", encrypted);

    let decrypted = xor_decrypt(encrypted.as_bytes(), key.as_bytes());
    println!("\nDecrypted: {:?}", decrypted);
}

fn xor_encrypt(shellcode: &[u8], key: &[u8]) -> String {
    let mut encrypted = Vec::new();
    for (i, &b) in shellcode.iter().enumerate() {
        encrypted.push(b ^ key[i % key.len()]);
    }
    base64::encode(&encrypted)
}

fn xor_decrypt(encrypted: &[u8], key: &[u8]) -> Vec<u8> {
    let encrypted = base64::decode(encrypted).expect("msg");

    let mut decrypted = Vec::new();
    for (i, &b) in encrypted.iter().enumerate() {
        decrypted.push(b ^ key[i % key.len()]);
    }
    decrypted
}
