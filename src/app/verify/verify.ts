import { Consecutive, verifyNotConsecutive } from './consecutive';
import { InARow, verifyNotInARow } from './inarow';
import { DoesntMeet, verifyMeets } from './meet';

export interface VerificationResult {
    doesntMeets: DoesntMeet[],
    inARows: InARow[],
    consecutives: Consecutive[],
}

export function verify(table: string[][][]): VerificationResult {
    return {
        doesntMeets: verifyMeets(table),
        inARows: verifyNotInARow(table),
        consecutives: verifyNotConsecutive(table),
    };
}