package com.zzq.demo11Shuzu;

public class randomValues {
    // 创建一个包含随机值的数组
    public static void main(String[] args) {
        double [] arr = new double[10];
        for (int i = 0; i < 10; i++) {
            arr[i] = Math.random();
        }
        System.out.println(arr);
    }
}
