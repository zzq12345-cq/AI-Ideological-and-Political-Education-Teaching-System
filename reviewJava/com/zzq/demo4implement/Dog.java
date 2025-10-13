package com.zzq.demo4implement;

public class Dog extends Animal implements Run{
    @Override
    void eat() {
        System.out.println("狗吃骨头");
    }

    @Override
    void sound() {
        System.out.println("汪汪!");
    }

    @Override
    public void run() {
        System.out.println("狗会跑");
    }
}
