package com.zzq.demo11Shuzu;

public class perLine {
    // 打印数组的值每行一个
    public static void main(String[] args) {
        double [] arr = new double[10];
        for (int i = 0; i < 10; i++){
            arr[i] = Math.random();
        }
        System.out.print(arr);
    }
}
