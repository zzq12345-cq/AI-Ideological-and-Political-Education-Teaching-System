package com.zzq.demo4implement;

public class Main {
    public static void main(String[] args) {
        Dog dog = new Dog();
        dog.eat();
        dog.sound();
        dog.run();
        System.out.println("==========================");
        Elephant elephant = new Elephant();
        elephant.eat();
        elephant.sound();
        elephant.swim();
        System.out.println("============================");
        Lion lion = new Lion();
        lion.eat();
        lion.sound();
        lion.capture();
        System.out.println("=============================");
        Deer deer = new Deer();
        deer.eat();
        deer.sound();
        deer.swim();
    }
}
