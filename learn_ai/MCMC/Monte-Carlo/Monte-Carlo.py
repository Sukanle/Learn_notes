import random
import math
import argparse
from statistics import mean, stdev

def monte_carlo_simulation(square_size: int, num_samples: int, iterations: int = 5):
    """
    蒙特卡洛模拟估算圆面积
    :param square_size: 正方形边长
    :param num_samples: 每次迭代采样数
    :param iterations: 迭代次数 (默认5次)
    :return: 统计结果字典
    """
    radius = square_size / 2.0
    real_area = math.pi * radius ** 2
    square_area = square_size ** 2
    
    predictions = []
    
    for iter in range(iterations):
        inside = 0
        
        # 生成随机点并统计
        for _ in range(num_samples):
            x = random.uniform(0, square_size)
            y = random.uniform(0, square_size)
            
            # 计算点与圆心的距离
            dx = x - radius
            dy = y - radius
            if math.hypot(dx, dy) <= radius:
                inside += 1
                
        # 计算预测面积
        predicted = (inside / num_samples) * square_area
        predictions.append(predicted)
        
        # 打印单次结果
        print(f"Iter {iter+1:2} => Predicted: {predicted:.2f} | Error: {real_area - predicted:+.2f}")

    # 计算统计指标
    avg_pred = mean(predictions)
    std_pred = stdev(predictions) if len(predictions) > 1 else 0.0
    min_pred = min(predictions)
    max_pred = max(predictions)
    
    return {
        "real_area": real_area,
        "average": avg_pred,
        "std_dev": std_pred,
        "min": min_pred,
        "max": max_pred,
        "samples": num_samples,
        "iterations": iterations
    }

if __name__ == "__main__":
    # 配置命令行参数解析
    parser = argparse.ArgumentParser(description="蒙特卡洛模拟估算圆面积")
    parser.add_argument("square", type=int, help="正方形边长")
    parser.add_argument("samples", type=int, help="每次迭代采样数")
    parser.add_argument("-i", "--iterations", type=int, default=5, 
                       help="迭代次数 (默认: 5)")
    
    args = parser.parse_args()
    
    # 参数验证
    if args.square <= 0 or args.samples <= 0 or args.iterations <= 0:
        print("参数必须为正整数")
        exit(1)
    
    # 执行模拟
    results = monte_carlo_simulation(args.square, args.samples, args.iterations)
    
    # 打印最终结果
    print("\n===== 最终结果 =====")
    print(f"理论面积:      {results['real_area']:.2f}")
    print(f"平均预测值:    {results['average']:.2f} (±{results['std_dev']:.2f})")
    print(f"最小值:        {results['min']:.2f}")
    print(f"最大值:        {results['max']:.2f}")
    print(f"迭代次数:      {results['iterations']}")
    print(f"总采样数:      {results['samples'] * results['iterations']:,}")
    print(f"绝对偏差:      {results['real_area'] - results['average']:+.2f}")
    print(f"相对误差:      {abs(results['real_area'] - results['average']) / results['real_area']:.2%}")
