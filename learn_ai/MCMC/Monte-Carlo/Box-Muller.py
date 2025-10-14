import numpy as np

def generate_normal(n):
    # 生成均匀分布的样本
    u1 = np.random.uniform(0, 1, n)
    u2 = np.random.uniform(0, 1, n)
    
    # Box-Muller变换
    z0 = np.sqrt(-2 * np.log(u1)) * np.cos(2 * np.pi * u2)
    z1 = np.sqrt(-2 * np.log(u1)) * np.sin(2 * np.pi * u2)
    
    # 合并结果
    return np.concatenate([z0, z1])

# 生成10000个正态分布样本
samples = generate_normal(5000)
print(f"生成样本示例（前5个）: {samples[:5]}")
print(f"均值: {np.mean(samples):.4f}, 标准差: {np.std(samples):.4f}")
