package calc;

import java.util.Stack;

/**
 * @author Helly Guo
 * <p>
 * Created on 10/8/19 9:44 AM
 */
public class A {

    private static final String EXPRESSION = "3*5-9*(38-7)";
    private static final String EXPRESSION_WITH_PARAM = "3*xyz-9*(yzx-7)+zxy";

    private static final C DD = new C(1, 1D, 9);
    private static final Stack<B.StackVal> SIMPLE_EXPRESSION = B.genExpression(EXPRESSION);
    private static final Stack<B.StackVal> COMPLEX_EXPRESSION = B.genExpression(EXPRESSION_WITH_PARAM);

    public static void main(String[] args) {
        int count = 0;
        while (count < 100000) {
            B.getResult(SIMPLE_EXPRESSION, String.class, null);
            B.getResult(COMPLEX_EXPRESSION, C.class, DD);
            count++;
        }
        try {
            Thread.sleep(60000);
        } catch (InterruptedException e) {
            //
        }
    }

}
