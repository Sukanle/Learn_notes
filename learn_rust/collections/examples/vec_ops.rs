//! Vec 常用操作速查，可与 `std::vector` 的算法对照。

fn main() {
    let mut v = vec![3, 1, 4, 1, 5, 9, 2, 6];

    v.sort(); // 稳定排序：std::stable_sort
    println!("sort: {v:?}");

    v.dedup(); // 去除相邻重复，需先排序
    println!("dedup: {v:?}");

    v.retain(|x| x % 2 == 0); // 类似 std::erase_if
    println!("retain(even): {v:?}");

    v.extend([10, 12]); // 类似 insert(end, ...)
    println!("extend: {v:?}");

    // 容量与预留：类比 vector::reserve
    let mut with_capacity = Vec::with_capacity(10);
    with_capacity.extend(0..5);
    println!(
        "len = {}, capacity = {}",
        with_capacity.len(),
        with_capacity.capacity()
    );

    // windows / chunks：只读切片视图，零分配
    println!("windows(3): {:?}", v.windows(3).collect::<Vec<_>>());
    println!("chunks(2): {:?}", v.chunks(2).collect::<Vec<_>>());

    // split_at_mut：同时获得两个可变切片（借用检查器允许的不相交借用）
    let (left, right) = v.split_at_mut(2);
    left[0] = 100;
    right[0] = 200;
    println!("split_at_mut: {v:?}");

    // 从迭代器构造：collect 需要标注目标类型
    let squares: Vec<i32> = (1..=5).map(|x| x * x).collect();
    println!("squares: {squares:?}");

    // drain：类似 erase(begin, end)，返回值仍可迭代
    let drained: Vec<i32> = v.drain(..2).collect();
    println!("drained = {drained:?}, remaining = {v:?}");

    // 查找与二分
    let sorted = vec![1, 3, 5, 7, 9];
    println!("contains(5) = {}", sorted.contains(&5));
    println!("binary_search(7) = {:?}", sorted.binary_search(&7));
    println!("position(>=5) = {:?}", sorted.iter().position(|&x| x >= 5));
}
