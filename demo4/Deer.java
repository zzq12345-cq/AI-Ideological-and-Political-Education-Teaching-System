package com.zzq.demo4implement;

public class Deer extends Animal implements Swim {
    @Override
    void eat() {
        System.out.println("鹿吃草");
    }

    @Override
    void sound() {
        System.out.println("嘀咕");
    }

    @Override
    public void swim() {
        System.out.println("鹿会游泳");
    }
}
