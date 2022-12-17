fn main() {
    
    const BUFFER_BYTES:&[u8] = include_bytes!("..\\..\\calc.bin");
    const BUFFER_SIZE:usize = BUFFER_BYTES.len();

    #[link_section = ".text"]
    static BUFFER:[u8;BUFFER_SIZE] = *include_bytes!("..\\..\\calc.bin");

    unsafe{
        let exec = std::mem::transmute::<*const u8,fn()>(&BUFFER as *const u8);
        exec();
    }
}
