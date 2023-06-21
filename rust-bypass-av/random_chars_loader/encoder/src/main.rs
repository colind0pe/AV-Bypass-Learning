fn main() {
    let buffer = include_bytes!("..\\..\\..\\calc.bin").as_slice();
    let key = "Vpe0YiGeYjKj5kMAsF0PjMEurwsMPZZY";
    println!("Key: {}", key);

    let random_string = add_random(encrypted.as_str(), key);
    println!("\n{}", random_string);

    let shellcode = rm_random(random_string.as_str());
    println!("\n{:?}", shellcode);
}

fn add_random(buffer: &str, key: &str) -> String {
    let mut result = String::new();

    for (i, c) in buffer.chars().enumerate() {
        result.push(c);
        result.push(key.chars().nth(i % key.len()).unwrap());
    }
    hex::encode(&result)
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
