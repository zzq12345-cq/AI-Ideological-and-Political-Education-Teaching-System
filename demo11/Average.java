package com.zzq.demo11Shuzu;

public class Average {
    public static void main(String[] args) {
        double [] arr = {1.2, 3.4, 5.6, 7.8, 9.0};
        double sum = 0;
        for (int i = 0; i < arr.length; i++){
            sum += arr[i];
        }
        double avg = sum / arr.length;
        System.out.println(avg);
    }
}
