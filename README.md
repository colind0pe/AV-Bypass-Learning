# AV-Bypass-Learning
免杀学习笔记

shellcode免杀的常用手法，实现绕过常见AV的效果

python部分代码来源于奇安信攻防社区文章：https://forum.butian.net/share/1690	作者：St3pBy

通过组合几种基础免杀方法，过国内常见的杀软是没问题的

但是部分杀软已经对pyinstaller打包出来的exe文件无差别报恶意文件，而且python打包出来的exe文件体积比较大，所以接下来也会尝试使用~~go语言~~C++将基础免杀思路实现一遍

C++部分代码来源于倾旋的博客: https://payloads.online/archivers/2019-11-10/1/ 	作者：倾旋

开始尝试用Rust写加载器...
