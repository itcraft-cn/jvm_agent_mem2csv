package demo;

import java.util.concurrent.atomic.AtomicLong;

public class A {

  public static void main(String[] args) {
    System.out.println("hello, world!");
    AtomicLong sum = new AtomicLong(0);
    for (int i = 1; i <= 100; i++) {
      for (int j = 1; j <= 100; j++) {
        sum.set(i & j);
      }
    }
  }
}
