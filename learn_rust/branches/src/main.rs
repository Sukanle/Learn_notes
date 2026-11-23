fn main() {
    'exit_program: loop {
        println!("输入:");
        println!("\t[0]：温度转换");
        println!("\t[1]：斐波那契数");
        println!("\t[2]：退出");
        println!("输入:");
        let mut input = String::new();
        std::io::stdin().read_line(&mut input).unwrap();

        let choice: u32 = match input.trim().parse() {
            Ok(num @ (0..3)) => num,
            Ok(_) => {
                println!("错误：请输入 0、1 或 2");
                continue;
            }
            Err(_) => {
                println!("错误：请输入数字");
                continue;
            }
        };

        match choice {
            0 => {
                let (temp, str) = tempconver();
                println!("温度转换结果：{}{}", temp, str);
            }
            1 => println!("斐波那契数结果：{}", inputfibonacci()),
            2 => break 'exit_program,
            _ => unreachable!(), // 已被上面过滤
        }
    }
    println!("退出程序");
}

fn tempconver() -> (f64, String) {
    let mut input = String::new();
    let choice = loop {
        input.clear();
        println!("输入:");
        println!("\t[0]：°C -> °F");
        println!("\t[1]：°F -> °C");
        std::io::stdin().read_line(&mut input).unwrap();
        match input.trim().parse() {
            Ok(n @ (0 | 1)) => break n,
            Ok(_) => println!("错误：请输入 0 或 1"),
            Err(_) => println!("错误：请输入选项数字"),
        }
    };
    let temp: f64 = loop {
        input.clear();
        println!("输入温度:");
        std::io::stdin().read_line(&mut input).unwrap();
        match input.trim().parse() {
            Ok(num) => break num,
            Err(_) => println!("错误：请输入数字"),
        }
    };
    match choice {
        0 => (temp * 9.0 / 5.0 + 32.0, "°F".to_string()), // ★ 公式 + 标签
        1 => ((temp - 32.0) * 5.0 / 9.0, "°C".to_string()),
        _ => unreachable!(), // 已被上面过滤
    }
}

fn inputfibonacci() -> usize {
    let mut input = String::new();
    fibonacci(loop {
        input.clear();
        println!("输入阶数:");
        std::io::stdin().read_line(&mut input).unwrap();
        match input.trim().parse() {
            Ok(num) => break num,
            Err(_) => {
                println!("错误：请输入数字");
                continue;
            }
        }
    })
}

fn fibonacci(n: u32) -> usize {
    let (mut a, mut b) = (0, 1);
    for _ in 0..n {
        (a, b) = (b, a + b);
    }
    return b;
}
