package com.zzq.demo2;

public class main {
    public static void main(String[] args) {
        Film[] films = new Film[4];
        films[0] = new Film(1,"西红柿首富",9.7,"沈腾");
        films[1] = new Film(2,"唐人街探案",9.5,"张译");
        films[2] = new Film(3,"唐人街探案2",9.5,"张译");
        films[3] = new Film(4,"唐人街探案3",9.5,"张译");

        FilmOperator fo = new FilmOperator(films);
        fo.printAllFilms();
        fo.search();
    }
}
