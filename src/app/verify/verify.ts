import { InARow, verifyNotInARow } from './inarow';
import { DoesntMeet, verifyMeets } from './meet';

export interface VerificationResult {
    doesntMeets: DoesntMeet[],
    inARows: InARow[],
}

export function verify(table: string[][][]): VerificationResult {
    return {
        doesntMeets: verifyMeets(table),
        inARows: verifyNotInARow(table),
    };
}