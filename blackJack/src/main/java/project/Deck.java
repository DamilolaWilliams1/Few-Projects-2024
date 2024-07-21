package project;

import java.util.ArrayList;
import java.util.Collections;
import java.util.List;
import java.util.Random;

public class Deck {

    private List<Card> cards;
    private Random random;

    public Deck() {
        cards = new ArrayList<>();
        random = new Random();
        reset();
    }

    public void reset() {
        cards.clear();
        for (String suit : Card.SUITS) {
            for (String face : Card.FACES) {
                cards.add(new Card(face, suit));
            }
        }
        Collections.shuffle(cards);
    }

    public Card dealCard() {
        if (cards.isEmpty()) {
            throw new RuntimeException("Deck is empty");
        }
        return cards.remove(cards.size() - 1);
    }
}
