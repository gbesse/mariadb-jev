-- Illustrative only. Assumes reviews(id, review, rating) and configured Jev UDFs.
-- The temporary table bounds provider calls independently of optimizer choices.
CREATE TEMPORARY TABLE candidate_reviews AS
SELECT id, review
FROM reviews
WHERE rating >= 3 AND review IS NOT NULL
ORDER BY id
LIMIT 5;

-- One paid request per uncached selected row; inspect probabilities manually.
SELECT id, jev_probability(review, 'The review recommends the movie') AS probability
FROM candidate_reviews;

DROP TEMPORARY TABLE candidate_reviews;
