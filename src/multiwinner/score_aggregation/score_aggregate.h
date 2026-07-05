#pragma once

enum ordinal { LESS = -1, EQUAL = 0, GREATER = 1 };

class score_aggregate {
	public:
		/*
		virtual bool operator<(const score_aggregate & other) const = 0;
		virtual bool operator<=(const score_aggregate & other) const = 0;
		virtual bool operator>(const score_aggregate & other) const = 0;
		virtual bool operator>=(const score_aggregate & other) const = 0;
		virtual bool operator==(const score_aggregate & other) const = 0;
		virtual bool operator!=(const score_aggregate & other) const = 0;
		*/

		virtual void insert(double weight, double score) = 0;
};