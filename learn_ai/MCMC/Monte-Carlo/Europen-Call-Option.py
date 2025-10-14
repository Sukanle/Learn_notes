import numpy as np

# 期权参数
S0 = 100  # 当前每股单价
K = 110   # 执行价格,即预售锁定的定金,相当于倒爷赚差价
T = 1     # 期权到期时间（以年计）,时间越长价格越容易有较大波动（无法确保是否一定是想要的波动）
r = 0.05  # 无风险利率（年化）,类似国债利率
sigma = 0.2  # 波动率（年化）,越大期权价值越高（大涨大跌可能性增加）
num_simulations = 100000  # 模拟路径数量

# 生成标准正态随机变量
Z = np.random.randn(num_simulations)

# 计算到期时的股票价格 S_T
ST = S0 * np.exp((r - 0.5 * sigma**2) * T + sigma * np.sqrt(T) * Z)

# 计算看涨期权的收益（max(ST - K, 0)）
payoffs = np.maximum(ST - K, 0)

# 计算期权价格（贴现到当前）
C0 = np.exp(-r * T) * np.mean(payoffs)

print(f"蒙特卡洛估算的欧式看涨期权价格: {C0:.4f}")
