package com.zzq.demo5callback;

public class main {
    public static void main(String[] args) {
        Chef chef = new Chef();
        System.out.println("我：叫厨师开始做饭");
        chef.cook(new Callback() { // 匿名内部类
            @Override
            public void complete() {
                System.out.println("我：收到通知，准备开饭");
            }
        });
    }
}
