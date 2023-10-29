package calc;

import java.lang.reflect.InvocationTargetException;
import java.lang.reflect.Method;
import java.util.ArrayList;
import java.util.Collections;
import java.util.HashMap;
import java.util.List;
import java.util.Map;
import java.util.Stack;
import java.util.concurrent.ConcurrentHashMap;

/**
 * 抽象导出类 <br>
 * Copyright <a href="http://www.erayt.com/" target="_blank">erayt.com</a>(c)<br>
 * <p>
 * 0.2.2版本仅支持加法/减法<br>
 * 0.2.3版本支持四则运算<br>
 *
 * @author GuoZheyong
 * @version 2012-1-6
 */
public class B {

    private static final String STR_ADD = "+";
    private static final String STR_SUB = "-";
    private static final String STR_MUL = "*";
    private static final String STR_DIV = "/";
    private static final String STR_LEFT_BRACKET = "(";
    private static final String STR_RIGHT_BRACKET = ")";
    private static final char CH_ADD = '+';
    private static final char CH_SUB = '-';
    private static final char CH_NEGATIVE_SIGN = CH_SUB;
    private static final char CH_MUL = '*';
    private static final char CH_DIV = '/';
    private static final char CH_LEFT_BRACKET = '(';
    private static final char CH_RIGHT_BRACKET = ')';
    private static final char CH_ZERO = '0';
    private static final char CH_NINE = '9';

    /**
     * 操作符动作表
     */
    private static final Map<String, OpAction> OP_ACTION;

    /**
     * 操作符权值表
     */
    private static final Map<String, Integer> OP_POWER;
    private static final int EXP_INITIAL_CAPACITY = 32;
    private static final Map<String, Method> METHOD_CACHE = new ConcurrentHashMap<>();

    static {
        OP_ACTION = new HashMap<>();
        OP_ACTION.put(STR_ADD, B::add);
        OP_ACTION.put(STR_SUB, B::sub);
        OP_ACTION.put(STR_MUL, B::mul);
        OP_ACTION.put(STR_DIV, B::div);
        OP_POWER = new HashMap<>();
        OP_POWER.put(STR_ADD, 0);
        OP_POWER.put(STR_SUB, 0);
        OP_POWER.put(STR_MUL, 1);
        OP_POWER.put(STR_DIV, 1);
        OP_POWER.put(STR_LEFT_BRACKET, 1);
        OP_POWER.put(STR_RIGHT_BRACKET, 0);
    }

    private B() {
    }

    /**
     * 根据逆波兰式栈生成结果集
     *
     * @param expression 可转换为Stack的逆波兰式栈
     * @param clazz      类
     * @param obj        类对象
     * @return 结果
     */
    public static Number getResult(Stack<StackVal> expression, Class<?> clazz, Object obj) {
        Stack<StackVal> expStack = new Stack<>();
        expStack.addAll(expression);
        // 临时数据栈
        Stack<StackVal> resultStack = new Stack<>();
        // 直到原栈中再没有数据或操作符
        while (!expStack.isEmpty()) {
            StackVal val = expStack.pop();
            if (StackType.OPERATOR == val.getType()) {
                // 为操作符
                operate(clazz, obj, resultStack, val.getOperator());
            } else {
                // 操作数先压入数据栈
                resultStack.push(val.cloneOne());
            }
        }
        return resultStack.pop().getNum();
    }

    private static void operate(Class<?> clazz, Object obj, Stack<StackVal> resultStack, String op) {
        // 值1
        // 后进先出，以支持减法、除法这些不支持交换律的方法
        Number value2 = getValue(resultStack, clazz, obj);
        // 值2
        // 后进先出，以支持减法、除法这些不支持交换律的方法
        Number value1 = getValue(resultStack, clazz, obj);
        // 根据操作符执行操作
        OpAction opAction = OP_ACTION.get(op);
        if (opAction == null) {
            // 不支持其他操作符
            throw new RuntimeException("不支持的操作符");
        } else {
            resultStack.push(new StackVal(StackType.NUMBER, opAction.operate(value1, value2)));
        }
    }

    /**
     * 加法
     *
     * @param value1 值1
     * @param value2 值2
     * @return 和
     */
    private static Number add(Number value1, Number value2) {
        boolean f1 = checkIsDouble(value1);
        boolean f2 = checkIsDouble(value2);
        if (f1 || f2) {
            Double d1 = f1 ? (Double) value1 : (Integer) value1 * 1.0D;
            Double d2 = f2 ? (Double) value2 : (Integer) value2 * 1.0D;
            return d1 + d2;
        } else {
            Integer i1 = (Integer) value1;
            Integer i2 = (Integer) value2;
            return i1 + i2;
        }
    }

    /**
     * 减法
     *
     * @param value1 值1
     * @param value2 值2
     * @return 差
     */
    private static Number sub(Number value1, Number value2) {
        boolean f1 = checkIsDouble(value1);
        boolean f2 = checkIsDouble(value2);
        if (f1 || f2) {
            Double d1 = f1 ? (Double) value1 : (Integer) value1 * 1.0D;
            Double d2 = f2 ? (Double) value2 : (Integer) value2 * 1.0D;
            return d1 - d2;
        } else {
            Integer i1 = (Integer) value1;
            Integer i2 = (Integer) value2;
            return i1 - i2;
        }
    }

    /**
     * 乘法
     *
     * @param value1 值1
     * @param value2 值2
     * @return 积
     */
    private static Number mul(Number value1, Number value2) {
        boolean f1 = checkIsDouble(value1);
        boolean f2 = checkIsDouble(value2);
        if (f1 || f2) {
            Double d1 = f1 ? (Double) value1 : (Integer) value1 * 1.0D;
            Double d2 = f2 ? (Double) value2 : (Integer) value2 * 1.0D;
            return d1 * d2;
        } else {
            Integer i1 = (Integer) value1;
            Integer i2 = (Integer) value2;
            return i1 * i2;
        }
    }

    /**
     * 除法
     *
     * @param value1 值1
     * @param value2 值2
     * @return 商
     */
    private static Number div(Number value1, Number value2) {
        boolean f1 = checkIsDouble(value1);
        boolean f2 = checkIsDouble(value2);
        Double d1 = f1 ? (Double) value1 : (Integer) value1 * 1.0D;
        Double d2 = f2 ? (Double) value2 : (Integer) value2 * 1.0D;
        return d1 / d2;
    }

    /**
     * 检查是否为double
     *
     * @param number 值
     * @return true/false
     */
    private static boolean checkIsDouble(Number number) {
        return Double.TYPE.isInstance(number) || number instanceof Double;
    }

    /**
     * 将值栈中第一个数据进行处理
     *
     * @param expressions 值栈
     * @param clazz       类
     * @param obj         类对象
     * @return 值
     */
    private static Number getValue(Stack<StackVal> expressions, Class<?> clazz,
                                   Object obj) {
        StackVal val = expressions.pop();
        // 检查是String还是方法还是数值
        if (StackType.NUMBER == val.getType()) {
            // 数值，直接返回
            return val.getNum();
        } else if (StackType.STRING == val.getType()) {
            // 字串
            return convertNum(val.getStr());
        } else {
            // 方法名，调用方法获取值
            return invokeMethod(clazz, obj, val.getMethod());
        }
    }

    private static Number convertNum(String ex) {
        // 数字模式
        // 转int先
        try {
            return Integer.parseInt(ex);
        } catch (Exception e) {
            //
        }
        // 尝试转double
        try {
            return Double.parseDouble(ex);
        } catch (Exception e) {
            //
            throw new RuntimeException(e.getMessage(), e);
        }
    }

    /**
     * 调用方法，均应是标准JavaBean的get方法
     *
     * @param clazz      类
     * @param target     类对象
     * @param methodName 方法名
     * @return 值
     */
    private static Number invokeMethod(Class<?> clazz, Object target,
                                       String methodName) {
        Method method = getMethod(clazz, methodName);
        try {
            return (Number) method.invoke(target);
        } catch (IllegalAccessException | InvocationTargetException e) {
            //
            throw new RuntimeException(e.getMessage(), e);
        }
    }

    private static Method getMethod(Class<?> clazz, String methodName) {
        String key = clazz.getName() + methodName;
        if (METHOD_CACHE.containsKey(key)) {
            return METHOD_CACHE.get(key);
        } else {
            Method method;
            try {
                method = clazz.getMethod(methodName);
            } catch (NoSuchMethodException e) {
                //
                throw new RuntimeException(e);
            }
            METHOD_CACHE.put(key, method);
            return method;
        }
    }

    public static Stack<StackVal> genExpression(String inputString) {
        List<String> list = new ArrayList<>(EXP_INITIAL_CAPACITY);
        char[] chars = inputString.toCharArray();
        // 为true表明：触碰到左括号，且未触碰到右括号
        boolean hitLeft = false;
        StringBuilder sb = new StringBuilder();
        for (int i = 0; i < chars.length; i++) {
            hitLeft = loopParse(chars[i], hitLeft, i == 0, list, sb);
        }
        if (sb.length() > 0) {
            // 结束循环后尚有值在字串中
            list.add(sb.toString());
            sb.setLength(0);
        }
        // 返回经中缀表达式处理所得的逆波兰式
        return convertToReversePolishNotation(list);
    }

    private static boolean loopParse(char ch, boolean hitLeft, boolean first, List<String> list, StringBuilder sb) {
        if ((first || hitLeft) && ch == CH_NEGATIVE_SIGN) {
            // 表明首数字是负数
            // 负号拼入字串中
            return appendToNum(sb, ch);
        } else if (ch == CH_ADD || ch == CH_SUB || ch == CH_MUL || ch == CH_DIV) {
            // 操作符
            return handleOther(sb, ch, list, false);
        } else if (ch == CH_LEFT_BRACKET) {
            // 左括号
            return handleOther(sb, ch, list, true);
        } else if (ch == CH_RIGHT_BRACKET) {
            // 右括号
            return handleOther(sb, ch, list, false);
        } else {
            // 其他字符
            // 拼入字串
            return appendToNum(sb, ch);
        }
    }

    private static boolean handleOther(StringBuilder sb, char ch, List<String> list, boolean val) {
        if (sb.length() > 0) {
            list.add(sb.toString());
            sb.setLength(0);
        }
        list.add(Character.toString(ch));
        return val;
    }

    private static boolean appendToNum(StringBuilder sb, char ch) {
        sb.append(ch);
        return false;
    }

    /**
     * 处理中缀表达式，转为逆波兰式
     *
     * @param list 中缀表达式列表
     * @return 逆波兰式值栈
     */
    private static Stack<StackVal> convertToReversePolishNotation(
            List<String> list) {
        // 最终栈
        Stack<StackVal> finalStack = new Stack<>();
        // 操作符栈
        Stack<String> opStack = new Stack<>();
        // 根据中缀表达式列表迭代
        for (String opOrVal : list) {
            loopCalc(opOrVal, opStack, finalStack);
        }
        // 将操作符栈中的操作符逐一弹出，压入最终栈
        while (!opStack.empty()) {
            finalStack.push(new StackVal(StackType.OPERATOR, opStack.pop()));
        }
        // 反转最终栈，得到正确的逆波兰式，原序为逆序
        Collections.reverse(finalStack);
        return finalStack;
    }

    private static void loopCalc(String opOrVal, Stack<String> opStack, Stack<StackVal> finalStack) {
        // 值或操作符
        if (OP_ACTION.containsKey(opOrVal)) {
            handleOperator(opStack, opOrVal, finalStack);
        } else if (STR_LEFT_BRACKET.equals(opOrVal)) {
            // 左括号
            opStack.push(opOrVal);
        } else if (STR_RIGHT_BRACKET.equals(opOrVal)) {
            // 右括号
            pushToFinal(opStack, finalStack);
        } else {
            // 值
            handleVal(finalStack, opOrVal);
        }
    }

    private static void handleOperator(Stack<String> opStack, String opOrVal, Stack<StackVal> finalStack) {
        // 操作符
        if (opStack.empty()) {
            // 为空，直接放入操作符栈
            opStack.push(opOrVal);
        } else {
            // 如果操作符栈不为空
            // 获取本操作符权值
            int power = OP_POWER.get(opOrVal);
            while (true) {
                if (pushToFinal(opStack, opOrVal, power, finalStack)) {
                    break;
                }
            }
        }
    }

    private static boolean pushToFinal(Stack<String> opStack, String opOrVal, int power, Stack<StackVal> finalStack) {
        // 操作符栈为空了，直接放入操作符栈结束
        if (opStack.empty()) {
            opStack.push(opOrVal);
            return true;
        }
        // 当前操作符栈顶操作符
        String topOp = opStack.peek();
        if (STR_LEFT_BRACKET.equals(topOp)) {
            // 栈顶为左括号，直接放入结束
            opStack.push(opOrVal);
            return true;
        }
        // 栈顶操作符权值
        int topPower = OP_POWER.get(topOp);
        if (power > topPower) {
            // 权值比栈顶的高，则推入栈结束
            opStack.push(opOrVal);
            return true;
        } else {
            // 权值与栈顶的相等或低，出栈顶操作符到最终栈，继续循环
            finalStack.push(new StackVal(StackType.OPERATOR, opStack.pop()));
        }
        return false;
    }

    private static void pushToFinal(Stack<String> opStack, Stack<StackVal> finalStack) {
        // 弹出操作符到最终栈，直到碰到左括号或者操作符栈为空（后者一般不会发生）
        String op;
        while (!opStack.empty()) {
            op = opStack.pop();
            if (STR_LEFT_BRACKET.equals(op)) {
                break;
            }
            finalStack.push(new StackVal(StackType.OPERATOR, op));
        }
    }

    private static void handleVal(Stack<StackVal> finalStack, String opOrVal) {
        // 数字直接压入最终栈
        // 字段，转为get方法名，转为方法名后，压入最终栈
        finalStack.push(checkIsNum(opOrVal)
                ? new StackVal(StackType.NUMBER, convertNum(opOrVal))
                : new StackVal(StackType.METHOD, genGetMethodName(opOrVal)));
    }

    /**
     * 根据字段名获得标准JavaBean的get方法名 <br>
     * 或保留原值(为数字或rowNum)
     *
     * @param propName 字段名
     * @return 方法名
     */
    private static String genGetMethodName(String propName) {
        // 不是数字，处理为字段，获得方法名
        return "get" + propName.substring(0, 1).toUpperCase() +
                propName.substring(1);
    }

    /**
     * 是数字
     *
     * @param input 输入
     * @return 是数字
     */
    private static boolean checkIsNum(String input) {
        char first = input.charAt(0);
        // 首字符为-或数字，认定为是数字
        return first >= CH_ZERO && first <= CH_NINE || first == CH_NEGATIVE_SIGN;
    }

    private enum StackType {
        /**
         * 普通字符串
         */
        STRING,
        /**
         * 已知是数值
         */
        NUMBER,
        /**
         * 方法名
         */
        METHOD,
        /**
         * 操作符
         */
        OPERATOR;
    }

    private interface OpAction {
        Number operate(Number v1, Number v2);
    }

    public static class StackVal implements Cloneable {
        private StackType type;
        private String str;
        private Number num;
        private String operator;
        private String method;

        private StackVal() {
        }

        public StackVal(StackType type, String str) {
            this();
            this.type = type;
            if (type == StackType.STRING) {
                this.str = str;
            } else if (type == StackType.OPERATOR) {
                this.operator = str;
            } else {
                this.method = str;
            }
        }

        public StackVal(StackType type, Number num) {
            this();
            this.type = type;
            this.num = num;
        }

        public StackType getType() {
            return type;
        }

        public String getStr() {
            return str;
        }

        public Number getNum() {
            return num;
        }

        public String getOperator() {
            return operator;
        }

        public String getMethod() {
            return method;
        }

        protected StackVal cloneOne() {
            StackVal val = new StackVal();
            val.type = this.type;
            val.str = this.str;
            val.num = this.num;
            val.operator = this.operator;
            val.method = this.method;
            return val;
        }

        @Override
        public String toString() {
            switch (type) {
                case STRING:
                    return "StackVal{" +
                            "type=" + type +
                            ", str='" + str + '\'' +
                            '}';
                case NUMBER:
                    return "StackVal{" +
                            "type=" + type +
                            ", num=" + num +
                            '}';
                case OPERATOR:
                    return "StackVal{" +
                            "type=" + type +
                            ", operator='" + operator + '\'' +
                            '}';
                case METHOD:
                    return "StackVal{" +
                            "type=" + type +
                            ", method='" + method + '\'' +
                            '}';
                default:
                    return "";
            }
        }

        @Override
        public StackVal clone() {
            try {
                return (StackVal) super.clone();
            } catch (CloneNotSupportedException e) {
                throw new RuntimeException(e);
            }
        }
    }
}
