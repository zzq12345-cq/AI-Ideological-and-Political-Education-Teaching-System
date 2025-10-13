package com.zzq.demo8CallBack;

public class Main {
    public static void main(String[] args) {
        System.out.println("程序开始了，我遇到难题");
        // 创建学霸实例
        StudeyBa ba = new StudeyBa();
        System.out.println("【我】去找学霸帮忙，并递给他一张写好指令的便利贴");
        // 调用学霸的方法
        ba.solveHardProblem(new onTaskCompleted() {
            @Override
            public void onTaskCompleted(String result) {
                System.out.println("（...学霸跑过来告诉我...）");
                System.out.println("【我】：太棒了！拿到了结果： " + result);
            }
        });
        System.out.println("【我】：学霸在计算了，我不用等他，我先做点别的简单题目...");
    }
}
