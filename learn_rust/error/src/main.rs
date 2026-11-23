// fn read_username_from_file -> Result<String, io::Error> {
//     fs::read_to_string("hello.txt")
// }

use std::io::Read;

fn main() -> Result<(), Box<dyn std::error::Error>> {
    let mut f = std::fs::File::open("hello.txt")?;
    let mut buf = String::new();
    f.read_to_string(&mut buf)?;
    println!("read file: {buf}");
    return Ok(());
}
