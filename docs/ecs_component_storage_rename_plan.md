# ECS 组件存储类重命名计划

## 目标

将现有的 `ComponentStorage<T>` 改名为 `DynamicComponentStorage<T>`，明确它属于运行时动态组件存储：组件实例可在运行时按实体添加、移除和查询，内部数据随实体数量动态增长。此命名也为后续引入静态组件存储保留清楚的区分空间。

本计划只重命名组件存储类型及其相关声明和引用，不改变 ECS 的存储结构、增删行为、迭代接口或运行结果。

## 当前职责与推荐命名

当前实现位于 `Engine/core/ecs/component_storage.h`：

- `ComponentStorageBase` 提供类型 ID 和多态析构，供 `ComponentManager` 的类型擦除容器持有。
- `ComponentStorage<T>` 按组件类型存储实体与组件数据，并通过映射支持运行时增删和查询。
- `ComponentManager` 在运行时注册、注销并查找各组件类型对应的存储。

建议命名映射：

| 当前名称 | 计划名称 | 说明 |
| --- | --- | --- |
| `ComponentStorageBase` | `DynamicComponentStorageBase` | 明确它是动态存储体系的类型擦除基类 |
| `ComponentStorage<T>` | `DynamicComponentStorage<T>` | 明确具体存储支持运行时动态管理 |
| `PositionComponentStorage` | `PositionDynamicComponentStorage`，或删除该别名 | 若仍需要别名，应遵循新命名；如无使用处可一并移除 |
| `component_storage.h` | `dynamic_component_storage.h` | 文件名与类型名保持一致 |

`ComponentManager` 暂不改名。它目前管理运行时注册的存储，但本次范围是存储类命名；是否将其改为 `DynamicComponentManager` 应作为单独决策，避免无必要地扩大改名范围。

## 涉及位置

1. `Engine/core/ecs/component_storage.h`
   - 重命名基类和模板类。
   - 更新继承关系、类内注释及别名。
   - 按计划将文件改名为 `dynamic_component_storage.h`。
2. `Engine/core/ecs/component_manager.h`
   - 更新头文件包含。
   - 更新 `data` 容器元素类型、`registerComponent<T>()` 和 `get<T>()` 的返回类型及转换。
3. 全仓库引用
   - 查找旧类型名和旧头文件名，更新调用端、测试、示例及文档。
   - 重点核对 `Engine/core/engine.cpp`、`Engine/core/engine.h` 以及其他 ECS 头文件。
4. Visual Studio 项目文件
   - 更新 `Engine/Engine.vcxproj` 与 `Engine/Engine.vcxproj.filters` 中的头文件路径（如果文件改名）。

## 实施步骤

1. 全仓库搜索 `ComponentStorage`、`component_storage.h` 和 `PositionComponentStorage`，记录所有引用。
2. 先重命名类型：基类改为 `DynamicComponentStorageBase`，模板类改为 `DynamicComponentStorage<T>`，保持数据成员和函数实现不变。
3. 更新 `ComponentManager` 的容器类型、注册和获取接口，确认类型擦除转换与基类析构仍正确。
4. 将头文件改名为 `dynamic_component_storage.h`，更新所有 `#include` 和 Visual Studio 项目/筛选器文件条目。
5. 对旧标识符做全仓库搜索，确认没有遗漏；若保留临时兼容别名，需明确其移除节点，本计划默认不保留兼容别名。
6. 构建 x64 Release，并运行现有 ECS 使用路径，确认注册、添加、查询、遍历和注销行为不变。

## 验收标准

- 代码中生产路径使用 `DynamicComponentStorage<T>` 与 `DynamicComponentStorageBase`。
- 不再存在旧头文件路径或旧类型名的无意引用；历史文档可按需要保留原称呼。
- `ComponentManager` 能正常注册、获取和注销多种组件存储。
- 组件的运行时添加、移除、查询和遍历行为与改名前一致。
- 按仓库 `AGENTS.md` 指南完成 x64 Release 构建。

## 风险与控制

- **类型转换遗漏：** `ComponentManager` 通过基类指针保存存储，基类和派生类的类型名都需一并更新。
- **工程文件遗漏：** Visual Studio 项目显式列出头文件，文件改名后需同步更新 `.vcxproj` 和 `.vcxproj.filters`。
- **范围扩大：** 本次不调整 `ComponentManager`、实体 ID、存储布局或 API 语义，只做命名与引用迁移。
- **别名冗余：** `PositionComponentStorage` 若无使用，可删除，避免保留与新命名不一致的概念。

## 推荐决策

采用 `DynamicComponentStorage<T>` / `DynamicComponentStorageBase`，并将头文件同步改为 `dynamic_component_storage.h`。该名称直观表达运行时动态管理，也便于将来与静态组件存储并列。
