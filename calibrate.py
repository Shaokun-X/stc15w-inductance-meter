import matplotlib.pyplot as plt

y_values = [330, 1000, 470, 10, 100, 220, 47, 22]
x_values = [11028556, 3737378, 7651872, 448800800, 42947912, 18379985, 88977800, 192227028]
x_values = [1/x for x in x_values]

if len(x_values) != len(y_values) or len(x_values) < 2:
    raise ValueError("x_values and y_values must contain the same number of points")

x_mean = sum(x_values) / len(x_values)
y_mean = sum(y_values) / len(y_values)

x_variance = sum((x - x_mean) ** 2 for x in x_values)
if x_variance == 0:
    raise ValueError("x_values must not all be equal")

slope = sum(
    (x - x_mean) * (y - y_mean) for x, y in zip(x_values, y_values)
) / x_variance
intercept = y_mean - slope * x_mean

print(f"y = {slope:.8f} * x + {intercept:.8f}")

line_x = [min(x_values), max(x_values)]
line_y = [slope * x + intercept for x in line_x]

plt.scatter(x_values, y_values, label="Calibration points")
plt.plot(line_x, line_y, color="tab:red", label="Linear regression")
plt.xlabel("x")
plt.ylabel("y")
plt.grid(True, alpha=0.3)
plt.legend()
plt.tight_layout()
plt.show()
