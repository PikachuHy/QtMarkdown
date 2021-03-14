# 轻量级标记语言 Markdown

## 起源

Markdown是[Daring Fireball](http://daringfireball.net/projects/markdown/syntax)的作者John Gruber发明，身为Blogger，可能他也觉得传统的HTML有些复杂吧，退一步来讲，若是一个毫无计算机基础的Blogger，根本也没必要去掌握那些复杂的标签语言知识。

## 发展

越来越多的软件和服务支持 Markdown 语法，应该说，Mac上大多数的写作软件都支持它。在线工具同样有很多，如果你的博客基于Wordpress或是 blogger，它同样支持发布。

不仅仅是写博客，一切文章都可以用Markdown语法来写，比如说你想将一个标题加大字体，只需要在相应文字前面加上一个#或是在它的下一行加上一些等号即可，还有比这更简单的调整格式方式吗？

## 语法参见

[官方语法说明](http://daringfireball.net/projects/markdown/syntax)

## 示例

---

## 标题

# 一级标题

## 二级标题

### 三级标题

#### 四级标题

##### 五级标题

###### 六级标题

## 强调

- **粗体**

- _斜体_

## 引用

> 区块引用

## 列表

- red
- green
- blue

1. red
2. green
3. blue

## 代码

代码的插入方式 `printf('\n');`

```
function method()
{
    alert("javascript");
}
```

```
class Test{
    public static void main(String argc[]){
        System.out.println("java");
    }
}
```

```
class Test{
    public static void main()
    {
        Console.WriteLine("C#");
    }
}
```

## 链接

行内连接 [GitHub](https://github.com/) 的链接

## 图片

![shiyanlou logo](https://static.shiyanlou.com/img/logo_03.png)
