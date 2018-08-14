/*************************************************************************
	> File Name: Student.java
	> Author: 
	> Mail: 
	> Created Time: Tue 14 Aug 2018 10:54:39 AM CST
 ************************************************************************/
package resource;
import resource.Person;

public class Student extends Person {
    private String school;
    private String className;
    private String studentNumber;

    public void getInfo() { 
        System.out.println(super.toString());
        System.out.println(this);
    }
    

    /* construct function */
    public Student() {}

    public Student(String name, int age, String sex,String address, String telephone ,String school, String className, String studentNumber) {
        super(name, age, sex, address, telephone);
        this.school = school;
        this.className = className;
        this.studentNumber = studentNumber;
    }


    @Override
    public String toString() {
        return "Student [school=" + school + ", className=" + className + ", studentNumber=" + studentNumber + "]";
    }
    /* define getter and setter*/

    public void setSchool(String school) { this.school = school; }
    public String getSchool() { return school; }

    public void setClassName(String className) { this.className = className; }
    public String getClassName() { return className; }

    public void setStudentNumber(String studentNumber) { this.studentNumber = studentNumber; }
    public String getStudentNumber() { return studentNumber; }
}


