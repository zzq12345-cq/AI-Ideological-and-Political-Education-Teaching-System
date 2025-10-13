package com.zzq.demo11Shuzu;

public class Max {
    // 打印数组最大值
    public static void main(String[] args) {
        double [] arr = {1.2, 3.4, 5.6, 7.8, 9.0};
        double max = arr[0];
        for (int i = 1; i < arr.length; i++){
            if (max < arr[i]){
                max = arr[i];
            }
        }
        System.out.println(max);
    }
}
