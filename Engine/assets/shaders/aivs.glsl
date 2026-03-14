#version 460

// 输入属性：顶点位置
layout(location = 0) in vec3 aPosition;
// 输入属性：顶点颜色（可选）
layout(location = 1) in vec4 aColor;

// 输出变量：传递给片段着色器的颜色
out vec4 vColor;

// 统一变量：模型视图投影矩阵
uniform mat4 uMVP;

void main() {
    // 将顶点位置乘以 MVP 矩阵，得到裁剪空间坐标
    gl_Position = uMVP * vec4(aPosition, 1.0);

    // 将输入的颜色直接传递给片段着色器
    vColor = aColor;
}