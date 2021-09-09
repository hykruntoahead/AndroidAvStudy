package com.ykhe.ffmpeg_study;

import org.junit.Test;

import java.util.ArrayList;
import java.util.Arrays;
import java.util.Collections;
import java.util.LinkedList;
import java.util.List;

import static org.junit.Assert.*;

/**
 * Example local unit test, which will execute on the development machine (host).
 *
 * @see <a href="http://d.android.com/tools/testing">Testing documentation</a>
 */
public class ExampleUnitTest {
    @Test
    public void addition_isCorrect() {
        assertEquals(4, 2 + 2);
    }

    static class ListNode{
        int val;
        ListNode next;
        ListNode(int val){this.val = val;}
    }

    @Test
    public void stackDemo(){
        ListNode listNode = new ListNode(1);
        listNode.next = new ListNode(4);
        listNode.next = new ListNode(5);
        listNode.next = new ListNode(7);

        reversePrint(listNode);
    }

    private int[] reversePrint(ListNode listNode) {
        LinkedList<Integer> integers = new LinkedList<>();
        while (listNode!=null){
            integers.addFirst(listNode.val);
            listNode = listNode.next;
        }

        int[] retArr = new int[integers.size()];

        for (int i = 0; i < integers.size(); i++) {
            retArr[i] = integers.get(i);
        }

        return retArr;
    }
}