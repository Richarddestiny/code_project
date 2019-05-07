/*************************************************************************
	> File Name: Extends.java
	> Author: 
	> Mail: 
	> Created Time: Tue 14 Aug 2018 02:57:27 PM CST
 ************************************************************************/
abstract class Father {
    private int money;

    public void setMoney(int money) { this.money = money; }
    public int getMoney() { return money; }
    public abstract void study();
}

interface A {
    public static final int i = 10;
    public abstract int getNum();
}

interface B {
    public String name = "Interface B";
    public abstract String getName();
}

class Son extends Father implements A,B {
    public int getNum() { return i; }
    public String getName() { return name; }
    public void study() { System.out.println(" I'm study!"); }
}


public class Extends {
    public static void main(String args[]) {
            Son s = new Son();

            System.out.println(s.getName());
            s.study();

    }
}
