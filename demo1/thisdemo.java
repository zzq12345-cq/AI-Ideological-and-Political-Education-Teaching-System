package com.zzq.demo1;

public class thisdemo {
    // this
    public static void main(String[] args) {
        Student s1 = new Student();
        s1.print();
        System.out.println(s1);

        Student s2 = new Student();
        s2.name = "周杰伦";
        s2.hobby("唱歌");

        Student s3 = new Student();
        s3.subject = "语文";
        s3.study("数学");
    }
}
