package com.zzq.demo1;

public class Student {
    String name;
    String subject;
    public void print(){
        System.out.println(this);
    }

    public void hobby(String name){
        System.out.println(this.name + "喜欢" + name ); //this用来解决变量名称冲突的问题
    }

    public void study(String subject){
        System.out.println(this.subject + "好过" + subject);
    }
}
