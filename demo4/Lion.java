package com.zzq.demo4implement;

public class Lion extends Animal implements Capture {

    @Override
    void eat() {
        System.out.println("狮子吃兔子");
    }

    @Override
    void sound() {
        System.out.println("吼叫");
    }

    @Override
    public void capture() {
        System.out.println("狮子会捕猎");
    }
}
