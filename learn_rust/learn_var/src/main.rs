fn main() {
    let old_tup = (100, 'A', true);
    println!("old_tup.1: {}", old_tup.1);
    let tup: (i32, f32, u8) = (114, 1.2, 42);
    println!("tup.0: {}", tup.0);

    let (x, y, z) = tup;
    println!("x: {}, y: {}, z: {}", x, y, z);

    let arr = [1, 2, 3, 4, 5];
    let b: [i32; 5] = [1, 2, 3, 4, 5];
    let c = [3; 5];

    println!("arr[0]: {}, b[4]: {}", arr[0], b[4]);
    println!("c: {:?}", c);
}
