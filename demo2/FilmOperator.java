package com.zzq.demo2;

import java.util.Scanner;

public class FilmOperator {
    Film[] films;
    public FilmOperator(Film[] films) {
        this.films = films;
    }

    public void printAllFilms() {
        for (int i = 0; i <films.length; i++) {
            Film f = films[i];
            System.out.println(f.getId() + "\t" + f.getName() + "\t" + f.getPrice() + "\t" + f.getActor());
        }
    }
    public void search() {
        System.out.println("id:");
        Scanner sc = new Scanner(System.in);
        int id = sc.nextInt();
        for (int i = 0; i < films.length; i++) {
            Film f = films[i];
            System.out.println(f.getId() + "\t" + f.getName() + "\t" + f.getPrice() + "\t" + f.getActor());
            return;
        }
    }
}
