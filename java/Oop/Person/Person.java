/*
 * Description:建立抽象类
 * 
 * Written By:Cai
 * 
 * Date Written:2017-10-17
 * 
 * */

package resource;

public abstract class Person {
    
    private String name; 
    private int age;   
    private String sex;   
    private String address;  
    private String telephone; 
      
    public Person() {}

    public Person(String name, int age, String sex, String address,
            String telephone) {
        
        this.name = name;
        this.age = age;
        this.sex = sex;
        this.address = address;
        this.telephone = telephone;
    }
    
    public String getName() {
        return name;
    }

    public void setName(String name) {
        this.name = name;
    }

    public int getAge() {
        return age;
    }

    public void setAge(int age) {
        this.age = age;
    }

    public String getSex() {
        return sex;
    }

    public void setSex(String sex) {
        this.sex = sex;
    }

    public String getAddress() {
        return address;
    }

    public void setAddress(String address) {
        this.address = address;
    }

    public String getTelephone() {
        return telephone;
    }

    public void setTelephone(String telephone) {
        this.telephone = telephone;
    }


    @Override
    public String toString() {
        return "Person [name=" + name + ", age=" + age + ", sex=" + sex
                + ", address=" + address + ", telephone=" + telephone + "]";
    }
    
    //使用模板模式，定义好简单模板，打印信息
    /*
     * 模板模式为面向对象设计模式之一，其大概思想(个人理解):
     * 在抽象方法中完成已经确定的内容，而不确定的部分，写成抽象方法并调用
     * 而抽象方法的具体实现，交给继承他的类去具体实现，
     * 这样开发人员只要写好自己的实现部分便可，然后在主方法中，
     * 通过实例化后的抽象类直接调用模板方法便可
     * 
     * 以下方法只是一个简单的表示
     *  
     * */
    public void display() {
        
       // System.out.println(this);              
        getInfo();                
        System.out.println( "本输出方法采用模板设计模式，以上为学生信息！" );   
    
    }

    public abstract void getInfo();

}
