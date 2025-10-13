package com.zzq.demo4implement;

import java.sql.SQLOutput;

public class Elephant extends Animal implements Swim {

    @Override
    void eat() {
        System.out.println("大象吃鱼");
    }

    @Override
    void sound() {
        System.out.println("呜呜!");
    }

    @Override
    public void swim() {
        System.out.println("大象会游泳");
    }
}
